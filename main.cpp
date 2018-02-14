#include <v8.h>
#include <nan.h>

using namespace v8;

#define JS_STR(...) Nan::New<v8::String>(__VA_ARGS__).ToLocalChecked()
#define JS_INT(val) Nan::New<v8::Integer>(val)
#define JS_NUM(val) Nan::New<v8::Number>(val)
#define JS_FLOAT(val) Nan::New<v8::Number>(val)
#define JS_BOOL(val) Nan::New<v8::Boolean>(val)

namespace rawBuffer {

class RawBuffer : public Nan::ObjectWrap {
public:
  static Handle<Object> Initialize() {
    Nan::EscapableHandleScope scope;

    // constructor
    Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
    ctor->InstanceTemplate()->SetInternalFieldCount(1);
    ctor->SetClassName(JS_STR("RawBuffer"));
    Nan::SetPrototypeMethod(ctor, "getArrayBuffer", RawBuffer::GetArrayBuffer);
    Nan::SetPrototypeMethod(ctor, "toAddress", RawBuffer::ToAddress);

    // prototype
    Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
    Nan::SetAccessor(proto, JS_STR("length"), RawBuffer::Length);

    Local<Function> ctorFn = ctor->GetFunction();

    Local<Function> fromAddressFn = Nan::New<Function>(RawBuffer::FromAddress);
    fromAddressFn->Set(JS_STR("RawBuffer"), ctorFn);
    ctorFn->Set(JS_STR("fromAddress"), fromAddressFn);

    return scope.Escape(ctorFn);
  }

protected:
  static NAN_METHOD(New) {
    Nan::HandleScope scope;

    Local<Object> rawBufferObj = info.This();

    if (info[0]->IsNumber()) {
      RawBuffer *rawBuffer = new RawBuffer(info[0]->Uint32Value());
      rawBuffer->Wrap(rawBufferObj);
    } else if (info[0]->IsArrayBuffer()) {
      RawBuffer *rawBuffer = new RawBuffer(Local<ArrayBuffer>::Cast(info[0]));
      rawBuffer->Wrap(rawBufferObj);
    } else {
      Nan::ThrowError("Invalid arguments");
    }

    info.GetReturnValue().Set(rawBufferObj);
  }
  static NAN_GETTER(Length) {
    RawBuffer *rawBuffer = ObjectWrap::Unwrap<RawBuffer>(info.This());

    Local<ArrayBuffer> arrayBuffer = Nan::New(rawBuffer->arrayBuffer);

    info.GetReturnValue().Set(Nan::New<Integer>((uint32_t)arrayBuffer->ByteLength()));
  }
  static NAN_METHOD(GetArrayBuffer) {
    RawBuffer *rawBuffer = ObjectWrap::Unwrap<RawBuffer>(info.This());

    Local<ArrayBuffer> arrayBuffer = Nan::New(rawBuffer->arrayBuffer);

    info.GetReturnValue().Set(arrayBuffer);
  }
  static NAN_METHOD(ToAddress) {
    RawBuffer *rawBuffer = ObjectWrap::Unwrap<RawBuffer>(info.This());

    Local<ArrayBuffer> arrayBuffer = Nan::New(rawBuffer->arrayBuffer);
    uintptr_t address = (uintptr_t)arrayBuffer->GetContents().Data();
    size_t size = arrayBuffer->ByteLength();

    Local<Array> array = Nan::New<Array>(2);
    array->Set(0, Nan::New<Number>(static_cast<double>(address)));
    array->Set(1, Nan::New<Number>(static_cast<double>(size)));

    info.GetReturnValue().Set(array);
  }
  static NAN_METHOD(FromAddress) {
    Local<Array> array = Local<Array>::Cast(info[0]);

    uintptr_t address = static_cast<uintptr_t>(array->Get(0)->NumberValue());
    size_t size = static_cast<uintptr_t>(array->Get(1)->NumberValue());
    Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), (void *)address, size);

    Local<Function> rawBufferConstructor = Local<Function>::Cast(info.Callee()->Get(JS_STR("RawBuffer")));
    Local<Value> argv[] = {
      arrayBuffer,
    };
    Local<Value> rawBufferObj = rawBufferConstructor->NewInstance(sizeof(argv) / sizeof(argv[0]), argv);

    info.GetReturnValue().Set(rawBufferObj);
  }

  RawBuffer() : arrayBuffer(Isolate::GetCurrent(), ArrayBuffer::New(Isolate::GetCurrent(), 0)) {}
  RawBuffer(size_t size) : arrayBuffer(Isolate::GetCurrent(), ArrayBuffer::New(Isolate::GetCurrent(), size)) {}
  RawBuffer(Local<ArrayBuffer> arrayBuffer) : arrayBuffer(Isolate::GetCurrent(), arrayBuffer) {}

private:
  Persistent<ArrayBuffer> arrayBuffer;
};

void Init(Handle<Object> exports) {
  exports->Set(JS_STR("RawBuffer"), RawBuffer::Initialize());
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)

}
