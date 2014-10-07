#include <cloudcv.hpp>

#include <framework/marshal/marshal.hpp>
#include <framework/marshal/node_object_builder.hpp>
#include <framework/Job.hpp>
#include <framework/NanCheck.hpp>
#include <framework/Logger.h>

#include "CameraCalibrationAlgorithm.hpp"

using namespace v8;
using namespace node;

namespace cloudcv
{

    class DetectPatternTask : public Job
    {
    public:
        DetectPatternTask(Local<Object> imageBuffer, cv::Size patternSize, PatternType type, NanCallback * callback)
            : Job(callback)
            , m_algorithm(patternSize, type)
        {
            TRACE_FUNCTION;
            NanAssignPersistent(mImageBuffer, imageBuffer);

            mImageData = Buffer::Data(imageBuffer);
            mImageDataLen = Buffer::Length(imageBuffer);
        }

        virtual ~DetectPatternTask()
        {
            TRACE_FUNCTION;
            NanDisposePersistent(mImageBuffer);
        }

    protected:

        void ExecuteNativeCode()
        {
            TRACE_FUNCTION;
            cv::Mat frame = cv::imdecode(cv::_InputArray(mImageData, mImageDataLen), cv::IMREAD_GRAYSCALE);
            if (frame.empty())
            {
                SetErrorMessage("Cannot decode input image");
                return;
            }

            try
            {
                m_patternfound = m_algorithm.detectCorners(frame, m_corners2d);                
            }
            catch (...)
            {
                SetErrorMessage("Internal exception");
            }
            
        }

        virtual Local<Value> CreateCallbackResult()
        {
            TRACE_FUNCTION;
            NanEscapableScope();
            Local<Object> res = NanNew<Object>(); //(Object::New());

            NodeObject resultWrapper(res);
            resultWrapper["patternFound"] = m_patternfound;

            if (m_patternfound)
            {
                resultWrapper["corners"] = m_corners2d;
            }

            return NanEscapeScope(res);
        }

    private:
        CameraCalibrationAlgorithm                      m_algorithm;
        CameraCalibrationAlgorithm::VectorOf2DPoints    m_corners2d;

        Persistent<Object>	 	 mImageBuffer;
        char                   * mImageData;
        int                      mImageDataLen;

        bool                     m_patternfound;
        bool                     m_returnImage;
    };

    class ComputeIntrinsicParametersTask : public Job
    {
    public:
        typedef CameraCalibrationAlgorithm::VectorOfVectorOf3DPoints VectorOfVectorOf3DPoints;
        typedef CameraCalibrationAlgorithm::VectorOfVectorOf2DPoints VectorOfVectorOf2DPoints;
        typedef CameraCalibrationAlgorithm::VectorOfMat              VectorOfMat;

        ComputeIntrinsicParametersTask(
            const std::vector<std::string>& files,
            cv::Size boardSize, 
            PatternType type, 
            NanCallback * callback
        )
            : Job(callback)
            , m_algorithm(boardSize, type)
            , m_imageFiles(files)
        {

        }

        ComputeIntrinsicParametersTask(
            const VectorOfVectorOf2DPoints& v,
            cv::Size imageSize,
            cv::Size boardSize, 
            PatternType type, 
            NanCallback * callback
        )
            : Job(callback)
            , m_algorithm(boardSize, type)
            , m_imageSize(imageSize)
            , m_gridCorners(v)
        {

        }

    protected:

        virtual void ExecuteNativeCode()
        {
            TRACE_FUNCTION;
            if (!m_imageFiles.empty()) {
                m_calibrationSuccess = m_algorithm.calibrateCamera(m_imageFiles, m_cameraMatrix, m_distCoeffs);                
            }
            else if (!m_gridCorners.empty()) {
                m_calibrationSuccess = m_algorithm.calibrateCamera(m_gridCorners, m_imageSize, m_cameraMatrix, m_distCoeffs);                
            }
            else {
                SetErrorMessage("Neither image files nor grid corners were passed");
            }

        }

        virtual Local<Value> CreateCallbackResult()
        {
            NanEscapableScope();
            Local<Object> res = NanNew<Object>(); //Local(Object::New());

            NodeObject resultWrapper(res);
            resultWrapper["intrinsic"]  = m_cameraMatrix;
            resultWrapper["distCoeffs"] = m_distCoeffs;

            return NanEscapeScope(res);
        }

    private:
        CameraCalibrationAlgorithm m_algorithm;
        std::vector<std::string>   m_imageFiles;
        cv::Size                   m_imageSize;
        VectorOfVectorOf2DPoints   m_gridCorners;
        cv::Mat                    m_cameraMatrix;
        cv::Mat                    m_distCoeffs;
        bool                       m_calibrationSuccess;
    };

    NAN_METHOD(calibrationPatternDetect)
    {
        TRACE_FUNCTION;
        NanEscapableScope();

        Local<Object>	imageBuffer;
        Local<Function> callback;
        cv::Size        patternSize;
        PatternType     pattern;
        std::string     error;
        LOG_TRACE_MESSAGE("Begin parsing arguments");
        if (NanCheck(args)
            .Error(&error)
            .ArgumentsCount(4)
            .Argument(0).IsBuffer().Bind(imageBuffer)
            .Argument(1).Bind(patternSize)
            .Argument(2).StringEnum<PatternType>({ 
                { "CHESSBOARD",     PatternType::CHESSBOARD }, 
                { "CIRCLES_GRID",   PatternType::CIRCLES_GRID }, 
                { "ACIRCLES_GRID",  PatternType::ACIRCLES_GRID } }).Bind(pattern)
            .Argument(3).IsFunction().Bind(callback))
        {
            LOG_TRACE_MESSAGE("Parsed function arguments");
            NanCallback *nanCallback = new NanCallback(callback);
            NanAsyncQueueWorker(new DetectPatternTask(imageBuffer, patternSize, pattern, nanCallback));
            NanReturnUndefined();
        }
        else if (!error.empty())
        {
            LOG_TRACE_MESSAGE(error);
            return NanThrowTypeError(error.c_str());
        }
    }

    NAN_METHOD(calibrateCamera)
    {
        TRACE_FUNCTION;
        NanEscapableScope();

        std::vector<std::string>                imageFiles;
        std::vector< std::vector<cv::Point2f> > imageCorners;
        Local<Function> callback;
        cv::Size        patternSize;
        cv::Size        imageSize;
        PatternType     pattern;
        std::string     error;

        if (NanCheck(args)
            .Error(&error)
            .ArgumentsCount(4)
            .Argument(0).IsArray().Bind(imageFiles)
            .Argument(1).Bind(patternSize)
            .Argument(2).StringEnum<PatternType>({ 
                { "CHESSBOARD",     PatternType::CHESSBOARD }, 
                { "CIRCLES_GRID",   PatternType::CIRCLES_GRID }, 
                { "ACIRCLES_GRID",  PatternType::ACIRCLES_GRID } }).Bind(pattern)
            .Argument(3).IsFunction().Bind(callback))
        {
            NanCallback *nanCallback = new NanCallback(callback);
            NanAsyncQueueWorker(new ComputeIntrinsicParametersTask(imageFiles, patternSize, pattern, nanCallback));
            NanReturnValue(NanTrue());
        } else if (NanCheck(args)
            .Error(&error)
            .ArgumentsCount(6)
            .Argument(0).IsArray().Bind(imageCorners)
            .Argument(1).Bind(imageSize)
            .Argument(2).Bind(patternSize)
            .Argument(3).StringEnum<PatternType>({ 
                { "CHESSBOARD",     PatternType::CHESSBOARD }, 
                { "CIRCLES_GRID",   PatternType::CIRCLES_GRID }, 
                { "ACIRCLES_GRID",  PatternType::ACIRCLES_GRID } }).Bind(pattern)
            .Argument(4).IsFunction().Bind(callback))
        {
            NanCallback *nanCallback = new NanCallback(callback);
            NanAsyncQueueWorker(new ComputeIntrinsicParametersTask(imageCorners, imageSize, patternSize, pattern, nanCallback));
            NanReturnValue(NanTrue());
        }
        else if (!error.empty())
        {
            return NanThrowTypeError(error.c_str());
        }
    }
}