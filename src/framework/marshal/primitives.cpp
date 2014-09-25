#include "marshal.hpp"

using namespace v8;

V8Result MarshalFromNative(int value)
{
	NanEscapableScope();
	return NanEscapeScope(NanNew<Int32>(value));
}

V8Result MarshalFromNative(unsigned int value)
{
	NanEscapableScope();
	return NanEscapeScope(NanNew<Uint32>(value));
}

V8Result MarshalFromNative(float value)
{
	NanEscapableScope();
    return NanEscapeScope(NanNew<Number>(value));
}

V8Result MarshalFromNative(double value)
{
	NanEscapableScope();
	return NanEscapeScope(NanNew<Number>(value));
}

V8Result MarshalFromNative(const char * value)
{
	NanEscapableScope();
	return NanEscapeScope(NanNew<String>(value));
}

void MarshalToNative(V8Result inVal, int& outVal)
{
    outVal = inVal->Int32Value();
}

void MarshalToNative(V8Result inVal, unsigned int& outVal)
{
    outVal = inVal->Uint32Value();
}

void MarshalToNative(V8Result inVal, float& outVal)
{
    outVal = static_cast<float>(inVal->NumberValue());
}

void MarshalToNative(V8Result inVal, double& outVal)
{
    outVal = inVal->NumberValue();
}