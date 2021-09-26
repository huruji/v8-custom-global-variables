#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include "deps/v8/include/libplatform/libplatform.h"
#include "deps/v8/include/v8.h"
using namespace v8;
using namespace std;

const char* version = "1.0.0";

v8::Local<v8::String> v8_str(const char* x) {
    return v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), x).ToLocalChecked();
}

void getVersion(const FunctionCallbackInfo<Value>& args) {
    args.GetReturnValue().Set(v8_str(version));
}

void XGetter(Local<String> property, const PropertyCallbackInfo<Value>& info) {
    info.GetReturnValue().Set(v8_str(version));
}

void XSetter(Local<String> property, Local<Value> value, const PropertyCallbackInfo<void>& info) {
}

class App {
public:
    std::unique_ptr<v8::Platform> platform;
    Isolate::CreateParams create_params;
    Isolate* isolate;
    v8::Local<v8::Context> context;
    Handle<ObjectTemplate> global;
public:
    void createPlatform(char* argv[]) {
        // v8 初始化
        V8::InitializeICUDefaultLocation(argv[0]);
        V8::InitializeExternalStartupData(argv[0]);
        this->platform = v8::platform::NewDefaultPlatform();
        V8::InitializePlatform(this->platform.get());
        V8::Initialize();
    }

    void createVM() {
        // 创建虚拟机
        this->create_params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();;
        this->isolate = v8::Isolate::New(this->create_params);
    }

    void ShutdownVM() {
        // 关闭虚拟机
        this->isolate->Dispose();
        V8::Dispose();
        V8::ShutdownPlatform();
        delete this->create_params.array_buffer_allocator;
    }

    Isolate* getIsolate() {
        return this->isolate;
    }

    void setUpGlobal() {
        this->global = v8::ObjectTemplate::New(this->isolate);
        this->global->SetAccessor(v8_str("__version"), XGetter, XSetter);
        this->global->Set(this->isolate, "getVersion", FunctionTemplate::New(this->isolate, getVersion));
    }


    char* readFile(const char* filename) {
        FILE* file = fopen(filename, "rb");
        if (file == NULL) {
            return new char[0];
        }
        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        rewind(file);
        char* chars = new char[size + 1];
        chars[size] = '\0';

        for (size_t i = 0; i < size;) {
            i += fread(&chars[i], 1, size - i, file);
            if (ferror(file)) {
                fclose(file);
                return new char[0];
            }
        }

        fclose(file);
        return chars;
    }

    void runScriptString(const char* scriptString) {
        // 进入 scope
        v8::Isolate::Scope isolate_scope(this->isolate);
        v8::HandleScope handle_scope(this->isolate);
        this->setUpGlobal();
        this->context = v8::Context::New(this->isolate, NULL, this->global);
        v8::Context::Scope context_scope(this->context);
        v8::Local<v8::String> source = v8_str(scriptString);
        // 编译运行
        v8::Local<v8::Script> script =
                v8::Script::Compile(context, source).ToLocalChecked();
        v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
        // 将结果转化为字符串
        v8::String::Utf8Value utf8(this->isolate, result);
        printf("%s\n", *utf8);
    }

    void runJSFile(const char* filename) {
        const char* scriptStr = this->readFile(filename);
        this->runScriptString(scriptStr);
    }

};


int main(int argc, char* argv[]) {
    App app;
    app.createPlatform(argv);
    app.createVM();
    if (argc < 2) {
        app.ShutdownVM();
        return 0;
    }

    const char* filename = argv[1];

    app.runJSFile(filename);

    app.ShutdownVM();
    return 0;
}