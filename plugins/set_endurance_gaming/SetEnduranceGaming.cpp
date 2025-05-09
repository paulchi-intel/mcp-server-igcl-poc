//  The MIT License
//
//  Copyright (C) 2025
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

#include <iostream>
#include <string>
#include <vector>
#include "PluginAPI.h"
#include "json.hpp"

// 請根據你的專案 include 對應的 IGCL API 標頭檔
#include <igcl_api.h>
#include <GenericIGCLApp.h>

using json = nlohmann::json;

// 工具描述
static PluginTool methods[] = {
    {
        "set_endurance_gaming_mode",
        "Set or cycle Endurance Gaming mode and control for a device.",
        R"({
            "$schema": "http://json-schema.org/draft-07/schema#",
            "type": "object",
            "properties": {
                "control": { "type": "integer", "description": "Control value: 0=OFF, 1=ON, 2=AUTO" },
                "mode": { "type": "integer", "description": "Mode value: 0=BETTER_PERFORMANCE, 1=BALANCED, 2=MAXIMUM_BATTERY" }
            },
            "required": ["control", "mode"],
            "additionalProperties": false
        })"
    }
};

const char* GetNameImpl() { return "endurance-gaming-tools"; }
const char* GetVersionImpl() { return "1.0.0"; }
PluginType GetTypeImpl() { return PLUGIN_TYPE_TOOLS; }

int InitializeImpl() {
    // 這裡可初始化 IGCL API
    return 1;
}

char* HandleRequestImpl(const char* req) {
    json request = json::parse(req);
    int control = request["params"]["arguments"]["control"].get<int>();
    int mode = request["params"]["arguments"]["mode"].get<int>();

    // 初始化 API
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

    // 只對第一個 device 設定
    ctl_endurance_gaming_t EG = {};
    ctl_3d_feature_getset_t Set3DProperty = { 0 };
    ctl_3d_feature_getset_t Get3DProperty = { 0 };
    Get3DProperty.Size = sizeof(Get3DProperty);
    Get3DProperty.FeatureType = CTL_3D_FEATURE_ENDURANCE_GAMING;
    Get3DProperty.bSet = FALSE;
    Get3DProperty.CustomValueSize = sizeof(ctl_endurance_gaming_t);
    Get3DProperty.pCustomValue = &EG;
    Get3DProperty.ValueType = CTL_PROPERTY_VALUE_TYPE_CUSTOM;
    Result = ctlGetSet3DFeature(hDevices[0], &Get3DProperty);

    // 設定新值
    EG.EGControl = static_cast<ctl_3d_endurance_gaming_control_t>(control);
    EG.EGMode = static_cast<ctl_3d_endurance_gaming_mode_t>(mode);

    Set3DProperty.Size = sizeof(Set3DProperty);
    Set3DProperty.FeatureType = CTL_3D_FEATURE_ENDURANCE_GAMING;
    Set3DProperty.bSet = TRUE;
    Set3DProperty.CustomValueSize = sizeof(ctl_endurance_gaming_t);
    Set3DProperty.pCustomValue = &EG;
    Set3DProperty.ValueType = CTL_PROPERTY_VALUE_TYPE_CUSTOM;
    Set3DProperty.Version = 0;

    Result = ctlGetSet3DFeature(hDevices[0], &Set3DProperty);

    json responseContent;
    responseContent["type"] = "text";
    responseContent["text"] = (Result == CTL_RESULT_SUCCESS)
        ? "Endurance Gaming mode/control set successfully."
        : "Failed to set Endurance Gaming mode/control.";

    json response;
    response["content"] = json::array();
    response["content"].push_back(responseContent);
    response["isError"] = (Result != CTL_RESULT_SUCCESS);

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
