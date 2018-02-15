#include <v8.h>
#include <nan.h>

using namespace v8;

#define JS_STR(...) Nan::New<v8::String>(__VA_ARGS__).ToLocalChecked()
#define JS_INT(val) Nan::New<v8::Integer>(val)
#define JS_NUM(val) Nan::New<v8::Number>(val)
#define JS_FLOAT(val) Nan::New<v8::Number>(val)
#define JS_BOOL(val) Nan::New<v8::Boolean>(val)

namespace rawBuffer {

void Init(Handle<Object> exports);

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
    Nan::SetPrototypeMethod(ctor, "peekAddress", RawBuffer::PeekAddress);

    // prototype
    Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
    Nan::SetAccessor(proto, JS_STR("length"), RawBuffer::Length);

    Local<Function> ctorFn = ctor->GetFunction();

    Local<Function> fromAddressFn = Nan::New<Function>(RawBuffer::FromAddress);
    fromAddressFn->Set(JS_STR("RawBuffer"), ctorFn);
    ctorFn->Set(JS_STR("fromAddress"), fromAddressFn);
    uintptr_t initFunctionAddress = (uintptr_t)Init;
    ctorFn->Set(JS_STR("initFunctionAddress"), Nan::New<Number>(*reinterpret_cast<double*>(&initFunctionAddress)));

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
      Local<ArrayBuffer> arrayBuffer = Local<ArrayBuffer>::Cast(info[0]);
      if (!arrayBuffer->IsExternal()) {
        arrayBuffer->Externalize();
      }
      RawBuffer *rawBuffer = new RawBuffer(arrayBuffer);
      rawBuffer->Wrap(rawBufferObj);
    } else {
      Nan::ThrowError("Invalid arguments");
    }

    info.GetReturnValue().Set(rawBufferObj);
  }
  static NAN_GETTER(Length) {
    RawBuffer *rawBuffer = ObjectWrap::Unwrap<RawBuffer>(info.This());

    Local<ArrayBuffer> arrayBuffer = Nan::New(rawBuffer->arrayBuffer);

    if (!arrayBuffer.IsEmpty()) {
      info.GetReturnValue().Set(Nan::New<Integer>((uint32_t)arrayBuffer->ByteLength()));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
  static NAN_METHOD(GetArrayBuffer) {
    RawBuffer *rawBuffer = ObjectWrap::Unwrap<RawBuffer>(info.This());

    Local<ArrayBuffer> arrayBuffer = Nan::New(rawBuffer->arrayBuffer);

    if (!arrayBuffer.IsEmpty()) {
      info.GetReturnValue().Set(arrayBuffer);
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
  static NAN_METHOD(ToAddress) {
    RawBuffer *rawBuffer = ObjectWrap::Unwrap<RawBuffer>(info.This());

    Local<ArrayBuffer> arrayBuffer = Nan::New(rawBuffer->arrayBuffer);

    if (!arrayBuffer.IsEmpty()) {
      uintptr_t address = (uintptr_t)arrayBuffer->GetContents().Data();
      size_t size = arrayBuffer->ByteLength();

      Local<Array> array = Nan::New<Array>(2);
      array->Set(0, Nan::New<Number>(*reinterpret_cast<double*>(&address)));
      array->Set(1, Nan::New<Number>(*reinterpret_cast<double*>(&size)));

      rawBuffer->arrayBuffer.Reset();

      info.GetReturnValue().Set(array);
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
  static NAN_METHOD(PeekAddress) {
    RawBuffer *rawBuffer = ObjectWrap::Unwrap<RawBuffer>(info.This());

    if (!rawBuffer->arrayBuffer.IsEmpty()) {
      Local<ArrayBuffer> arrayBuffer = Nan::New(rawBuffer->arrayBuffer);
      uintptr_t address = (uintptr_t)arrayBuffer->GetContents().Data();

      info.GetReturnValue().Set(Nan::New<Number>(*reinterpret_cast<double*>(&address)));
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
  static NAN_METHOD(FromAddress) {
    Local<Array> array = Local<Array>::Cast(info[0]);

    double addressValue = array->Get(0)->NumberValue();
    uintptr_t address = *reinterpret_cast<uintptr_t*>(&addressValue);
    double sizeValue = array->Get(1)->NumberValue();
    size_t size = *reinterpret_cast<uintptr_t*>(&sizeValue);
    Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), (void *)address, size);

    Local<Function> rawBufferConstructor = Local<Function>::Cast(info.Callee()->Get(JS_STR("RawBuffer")));
    Local<Value> argv[] = {
      arrayBuffer,
    };
    Local<Value> rawBufferObj = rawBufferConstructor->NewInstance(sizeof(argv)/sizeof(argv[0]), argv);

    info.GetReturnValue().Set(rawBufferObj);
  }

  RawBuffer(size_t size) : arrayBuffer(Isolate::GetCurrent(), ArrayBuffer::New(Isolate::GetCurrent(), new unsigned char[size], size)) {}
  RawBuffer(Local<ArrayBuffer> arrayBuffer) : arrayBuffer(Isolate::GetCurrent(), arrayBuffer) {}
  ~RawBuffer() {
    Local<ArrayBuffer> arrayBuffer = Nan::New(this->arrayBuffer);
    if (!arrayBuffer.IsEmpty() && arrayBuffer->IsExternal()) {
      free(arrayBuffer->GetContents().Data());
    }
  }

private:
  Persistent<ArrayBuffer> arrayBuffer;
};

void Init(Handle<Object> exports) {
  exports->Set(JS_STR("RawBuffer"), RawBuffer::Initialize());
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)

}
