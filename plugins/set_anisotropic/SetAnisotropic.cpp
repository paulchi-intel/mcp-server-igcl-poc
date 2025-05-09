//  The MIT License
//
//  Copyright (C) 2025 Your Name
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  'Software'), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be
//  included in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include <thread>
#include "PluginAPI.h"
#include "json.hpp"

#include <igcl_api.h>
#include <GenericIGCLApp.h>

using json = nlohmann::json;

struct AnisotropicMode {
    uint32_t flag;
    const char* name;
};

static const AnisotropicMode kAnisoModes[] = {
    {CTL_3D_ANISOTROPIC_TYPES_APP_CHOICE, "App Choice"},
    {CTL_3D_ANISOTROPIC_TYPES_2X, "2X"},
    {CTL_3D_ANISOTROPIC_TYPES_4X, "4X"},
    {CTL_3D_ANISOTROPIC_TYPES_8X, "8X"},
    {CTL_3D_ANISOTROPIC_TYPES_16X, "16X"}
};

static PluginTool methods[] = {
    {
        "set_anisotropic",
        "Set Anisotropic mode for a device.",
        R"({
            "$schema": "http://json-schema.org/draft-07/schema#",
            "type": "object",
            "properties": {
                "mode": { "type": "integer", "description": "Mode value: 0=APP_CHOICE, 1=2X, 2=4X, 3=8X, 4=16X" }
            },
            "required": ["mode"],
            "additionalProperties": false
        })"
    }
};

const char* GetNameImpl() { return "set-anisotropic"; }
const char* GetVersionImpl() { return "1.0.0"; }
PluginType GetTypeImpl() { return PLUGIN_TYPE_TOOLS; }

int InitializeImpl() { return 1; }

uint32_t GetModeFlag(int mode) {
    if (mode < 0 || mode > 4) return 0xFFFFFFFF;
    return kAnisoModes[mode].flag;
}
const char* GetModeNameByIndex(int mode) {
    if (mode < 0 || mode > 4) return "Unknown";
    return kAnisoModes[mode].name;
}

char* HandleRequestImpl(const char* req) {
    json request;
    int mode = -1;
    try {
        request = json::parse(req);
        if (request.contains("mode")) {
            mode = request.value("mode", -1);
        } else if (request.contains("params") && request["params"].contains("arguments") && request["params"]["arguments"].contains("mode")) {
            mode = request["params"]["arguments"].value("mode", -1);
        }
    } catch (...) {
        json responseContent;
        responseContent["type"] = "text";
        responseContent["text"] = "Invalid JSON request.";
        json response;
        response["content"] = json::array();
        response["content"].push_back(responseContent);
        response["isError"] = true;
        std::string result = response.dump();
        char* buffer = new char[result.length() + 1];
#ifdef _WIN32
        strcpy_s(buffer, result.length() + 1, result.c_str());
#else
        strcpy(buffer, result.c_str());
#endif
        return buffer;
    }
    uint32_t mode_flag = GetModeFlag(mode);
    if (mode_flag == 0xFFFFFFFF) {
        json responseContent;
        responseContent["type"] = "text";
        responseContent["text"] = "Unsupported mode. Mode value must be 0~4.";
        json response;
        response["content"] = json::array();
        response["content"].push_back(responseContent);
        response["isError"] = true;
        std::string result = response.dump();
        char* buffer = new char[result.length() + 1];
#ifdef _WIN32
        strcpy_s(buffer, result.length() + 1, result.c_str());
#else
        strcpy(buffer, result.c_str());
#endif
        return buffer;
    }

    // 1. 初始化 IGCL API
    ctl_result_t Result = CTL_RESULT_SUCCESS;
    ctl_device_adapter_handle_t* hDevices = nullptr;
    uint32_t AdapterCount = 0;
    ctl_init_args_t CtlInitArgs;
    ctl_api_handle_t hAPIHandle;

    ZeroMemory(&CtlInitArgs, sizeof(ctl_init_args_t));
    CtlInitArgs.AppVersion = CTL_MAKE_VERSION(CTL_IMPL_MAJOR_VERSION, CTL_IMPL_MINOR_VERSION);
    CtlInitArgs.flags = 0;
    CtlInitArgs.Size = sizeof(CtlInitArgs);
    CtlInitArgs.Version = 0;

    Result = ctlInit(&CtlInitArgs, &hAPIHandle);
    if (Result != CTL_RESULT_SUCCESS) {
        json responseContent;
        responseContent["type"] = "text";
        responseContent["text"] = "ctlInit failed";
        json response;
        response["content"] = json::array();
        response["content"].push_back(responseContent);
        response["isError"] = true;
        std::string result = response.dump();
        char* buffer = new char[result.length() + 1];
#ifdef _WIN32
        strcpy_s(buffer, result.length() + 1, result.c_str());
#else
        strcpy(buffer, result.c_str());
#endif
        return buffer;
    }

    // 2. Enumerate devices
    Result = ctlEnumerateDevices(hAPIHandle, &AdapterCount, hDevices);
    hDevices = (ctl_device_adapter_handle_t*)malloc(sizeof(ctl_device_adapter_handle_t) * AdapterCount);
    Result = ctlEnumerateDevices(hAPIHandle, &AdapterCount, hDevices);
    if (Result != CTL_RESULT_SUCCESS || AdapterCount == 0) {
        json responseContent;
        responseContent["type"] = "text";
        responseContent["text"] = "No device found";
        json response;
        response["content"] = json::array();
        response["content"].push_back(responseContent);
        response["isError"] = true;
        std::string result = response.dump();
        char* buffer = new char[result.length() + 1];
#ifdef _WIN32
        strcpy_s(buffer, result.length() + 1, result.c_str());
#else
        strcpy(buffer, result.c_str());
#endif
        return buffer;
    }

    // 3. 設定每個 device 的 Anisotropic mode
    std::ostringstream oss;
    for (uint32_t i = 0; i < AdapterCount; ++i) {
        ctl_3d_feature_getset_t Set3DProperty = { 0 };
        Set3DProperty.Size = sizeof(Set3DProperty);
        Set3DProperty.FeatureType = CTL_3D_FEATURE_ANISOTROPIC;
        Set3DProperty.bSet = TRUE;
        Set3DProperty.CustomValueSize = 0;
        Set3DProperty.pCustomValue = NULL;
        Set3DProperty.ValueType = CTL_PROPERTY_VALUE_TYPE_ENUM;
        Set3DProperty.Value.EnumType.EnableType = mode_flag;
        Set3DProperty.Version = 0;

        Result = ctlGetSet3DFeature(hDevices[i], &Set3DProperty);
        if (Result == CTL_RESULT_SUCCESS) {
            oss << "Device Index: " << i << "\n  Set Anisotropic Mode: " << GetModeNameByIndex(mode) << " (Flag: " << mode_flag << ")\n  Status: Success\n\n";
        } else {
            oss << "Device Index: " << i << "\n  Set Anisotropic Mode: " << GetModeNameByIndex(mode) << " (Flag: " << mode_flag << ")\n  Status: Failed\n\n";
        }
    }
    free(hDevices);

    std::string text = oss.str();
    if (text.empty()) text = "No device processed.";

    json responseContent;
    responseContent["type"] = "text";
    responseContent["text"] = text;

    json response;
    response["content"] = json::array();
    response["content"].push_back(responseContent);
    response["isError"] = false;

    std::string result = response.dump();
    char* buffer = new char[result.length() + 1];
#ifdef _WIN32
    strcpy_s(buffer, result.length() + 1, result.c_str());
#else
    strcpy(buffer, result.c_str());
#endif
    return buffer;
}

void ShutdownImpl() {}

int GetToolCountImpl() {
    return sizeof(methods) / sizeof(methods[0]);
}

const PluginTool* GetToolImpl(int index) {
    if (index < 0 || index >= GetToolCountImpl()) return nullptr;
    return &methods[index];
}

static PluginAPI plugin = {
    GetNameImpl,
    GetVersionImpl,
    GetTypeImpl,
    InitializeImpl,
    HandleRequestImpl,
    ShutdownImpl,
    GetToolCountImpl,
    GetToolImpl,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

extern "C" PLUGIN_API PluginAPI* CreatePlugin() {
    return &plugin;
}

extern "C" PLUGIN_API void DestroyPlugin(PluginAPI*) {
    // Nothing to clean up for this example
}
