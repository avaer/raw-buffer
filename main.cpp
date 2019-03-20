#include <v8.h>
#include <nan.h>

using namespace v8;

#define JS_STR(...) Nan::New<v8::String>(__VA_ARGS__).ToLocalChecked()
#define JS_INT(val) Nan::New<v8::Integer>(val)
#define JS_NUM(val) Nan::New<v8::Number>(val)
#define JS_FLOAT(val) Nan::New<v8::Number>(val)
#define JS_BOOL(val) Nan::New<v8::Boolean>(val)

#define TO_NUM(x) (Nan::To<double>(x).FromJust())
#define TO_BOOL(x) (Nan::To<bool>(x).FromJust())
#define TO_UINT32(x) (Nan::To<unsigned int>(x).FromJust())
#define TO_INT32(x) (Nan::To<int>(x).FromJust())
#define TO_FLOAT(x) (Nan::To<float>(x).FromJust())

namespace rawBuffer {

void Init(Local<Object> exports);

class RawBuffer : public Nan::ObjectWrap {
public:
  static Local<Object> Initialize() {
    Nan::EscapableHandleScope scope;

    // constructor
    Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(New);
    ctor->InstanceTemplate()->SetInternalFieldCount(1);
    ctor->SetClassName(JS_STR("RawBuffer"));
    Nan::SetPrototypeMethod(ctor, "getArrayBuffer", RawBuffer::GetArrayBuffer);
    Nan::SetPrototypeMethod(ctor, "toAddress", RawBuffer::ToAddress);
    Nan::SetPrototypeMethod(ctor, "detach", RawBuffer::Detach);
    Nan::SetPrototypeMethod(ctor, "equals", RawBuffer::Equals);

    // prototype
    Local<ObjectTemplate> proto = ctor->PrototypeTemplate();
    Nan::SetAccessor(proto, JS_STR("length"), RawBuffer::Length);

    Local<Function> ctorFn = Nan::GetFunction(ctor).ToLocalChecked();

    Local<Function> fromAddressFn = Nan::New<Function>(RawBuffer::FromAddress);
    ctorFn->Set(JS_STR("fromAddress"), fromAddressFn);

    uintptr_t initFunctionAddress = (uintptr_t)Init;
    Local<Array> initFunctionAddressArray = Nan::New<Array>(2);
    initFunctionAddressArray->Set(0, Nan::New<Integer>((uint32_t)(initFunctionAddress >> 32)));
    initFunctionAddressArray->Set(1, Nan::New<Integer>((uint32_t)(initFunctionAddress & 0xFFFFFFFF)));
    ctorFn->Set(JS_STR("initFunctionAddress"), initFunctionAddressArray);

    return scope.Escape(ctorFn);
  }

protected:
  static NAN_METHOD(New) {
    Nan::HandleScope scope;

    Local<Object> rawBufferObj = info.This();

    if (info[0]->IsNumber() && info[1]->IsNumber() && info[2]->IsNumber() && info[3]->IsNumber()) {
      uintptr_t address = ((uint64_t)TO_UINT32(info[0]) << 32) | ((uint64_t)TO_UINT32(info[1]));
      size_t size = ((uint64_t)TO_UINT32(info[2]) << 32) | ((uint64_t)TO_UINT32(info[3]));
      
      RawBuffer *rawBuffer = new RawBuffer(address, size);
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

    info.GetReturnValue().Set(Nan::New<Integer>((uint32_t)rawBuffer->size));
  }
  static NAN_METHOD(GetArrayBuffer) {
    RawBuffer *rawBuffer = ObjectWrap::Unwrap<RawBuffer>(info.This());

    if (rawBuffer->owned) {
      Local<ArrayBuffer> arrayBuffer = ArrayBuffer::New(Isolate::GetCurrent(), (void *)rawBuffer->address, rawBuffer->size);
      info.GetReturnValue().Set(arrayBuffer);
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
  static NAN_METHOD(ToAddress) {
    RawBuffer *rawBuffer = ObjectWrap::Unwrap<RawBuffer>(info.This());

    if (rawBuffer->owned) {
      Local<Array> array = Nan::New<Array>(4);
      array->Set(0, Nan::New<Integer>((uint32_t)(rawBuffer->address >> 32)));
      array->Set(1, Nan::New<Integer>((uint32_t)(rawBuffer->address & 0xFFFFFFFF)));
      array->Set(2, Nan::New<Integer>((uint32_t)(rawBuffer->size >> 32)));
      array->Set(3, Nan::New<Integer>((uint32_t)(rawBuffer->size & 0xFFFFFFFF)));

      rawBuffer->address = 0;
      rawBuffer->size = 0;
      rawBuffer->owned = false;

      info.GetReturnValue().Set(array);
    } else {
      info.GetReturnValue().Set(Nan::Null());
    }
  }
  static NAN_METHOD(Detach) {
    RawBuffer *rawBuffer = ObjectWrap::Unwrap<RawBuffer>(info.This());

    if (rawBuffer->owned) {
      rawBuffer->address = 0;
      rawBuffer->size = 0;
      rawBuffer->owned = false;
    }
  }
  static NAN_METHOD(Equals) {
    if (info[0]->IsArray()) {
      Local<Array> array = Local<Array>::Cast(info[0]);

      if (array->Get(0)->IsNumber() && array->Get(1)->IsNumber()) {
        RawBuffer *rawBuffer = ObjectWrap::Unwrap<RawBuffer>(info.This());

        info.GetReturnValue().Set(Nan::New<Boolean>(TO_UINT32(array->Get(0)) == (uint32_t)(rawBuffer->address >> 32) && TO_UINT32(array->Get(1)) == (uint32_t)(rawBuffer->address & 0xFFFFFFFF)));
      } else {
        info.GetReturnValue().Set(Nan::New<Boolean>(false));
      }
    } else {
      info.GetReturnValue().Set(Nan::New<Boolean>(false));
    }
  }
  static NAN_METHOD(FromAddress) {
    Local<Array> array = Local<Array>::Cast(info[0]);
    
    Local<Function> rawBufferConstructor = Local<Function>::Cast(info.This());
    Local<Value> argv[] = {
      array->Get(0),
      array->Get(1),
      array->Get(2),
      array->Get(3),
    };
    Local<Value> rawBufferObj = rawBufferConstructor->NewInstance(Isolate::GetCurrent()->GetCurrentContext(), sizeof(argv)/sizeof(argv[0]), argv).ToLocalChecked();

    info.GetReturnValue().Set(rawBufferObj);
  }

  RawBuffer(uintptr_t address, size_t size) : address(address), size(size) {}
  RawBuffer(Local<ArrayBuffer> arrayBuffer) : address((uintptr_t)arrayBuffer->GetContents().Data()), size(arrayBuffer->ByteLength()) {}
  ~RawBuffer() {
    if (owned) {
      free((void *)address);
    }
  }

private:
  uintptr_t address;
  size_t size;
  bool owned = true;
};

void Init(Local<Object> exports) {
  exports->Set(JS_STR("RawBuffer"), RawBuffer::Initialize());
}

}

#ifndef LUMIN
NODE_MODULE(NODE_GYP_MODULE_NAME, rawBuffer::Init)
#else
extern "C" {
  void node_register_module_raw_buffer(Local<Object> exports, Local<Value> module, Local<Context> context) {
    rawBuffer::Init(exports);
  }
}
#endif