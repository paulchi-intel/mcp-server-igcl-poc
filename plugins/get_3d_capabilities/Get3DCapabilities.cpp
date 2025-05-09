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

static PluginTool methods[] = {
    {
        "get_3d_capabilities",
        "取得所有裝置支援的3D功能能力 (Get supported 3D feature capabilities for all devices)",
        "{\"$schema\": \"http://json-schema.org/draft-07/schema#\", \"type\": \"object\", \"properties\": {}, \"additionalProperties\": false}"
    }
};

const char* GetNameImpl() { return "get-3d-capabilities"; }
const char* GetVersionImpl() { return "1.0.0"; }
PluginType GetTypeImpl() { return PLUGIN_TYPE_TOOLS; }

int InitializeImpl() {
    return 1;
}

// 參考 EnduranceGaming.cpp 的 hDevice 取得方式與 reference/3D_Feature_Sample_App.cpp 的 CtlGet3DFeatureCaps 實作
char* HandleRequestImpl(const char* req) {
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

    // 3. 查詢每個 device 的 3D capabilities
    std::ostringstream oss;
    for (uint32_t i = 0; i < AdapterCount; ++i) {
        ctl_3d_feature_caps_t FeatureCaps3D = { 0 };
        FeatureCaps3D.Size = sizeof(ctl_3d_feature_caps_t);
        Result = ctlGetSupported3DCapabilities(hDevices[i], &FeatureCaps3D);
        if (Result != CTL_RESULT_SUCCESS) continue;
        FeatureCaps3D.pFeatureDetails = (ctl_3d_feature_details_t*)malloc(sizeof(ctl_3d_feature_details_t) * FeatureCaps3D.NumSupportedFeatures);
        if (!FeatureCaps3D.pFeatureDetails) continue;
        memset(FeatureCaps3D.pFeatureDetails, 0x0, sizeof(ctl_3d_feature_details_t) * FeatureCaps3D.NumSupportedFeatures);
        Result = ctlGetSupported3DCapabilities(hDevices[i], &FeatureCaps3D);
        if (Result == CTL_RESULT_SUCCESS) {
            oss << "Device Index: " << i << "\n";
            for (uint32_t j = 0; j < FeatureCaps3D.NumSupportedFeatures; ++j) {
                const auto& detail = FeatureCaps3D.pFeatureDetails[j];
                oss << "  Feature: " << Get3DFeatureName(detail.FeatureType) << " (Type: " << detail.FeatureType << ")\n";
                oss << "    Value Type: " << detail.ValueType << "\n";
                oss << "    Custom Value Size: " << detail.CustomValueSize << "\n";
                oss << "    Per App Support: " << (detail.PerAppSupport ? "Yes" : "No") << "\n";
                oss << "    Conflicting Features: " << detail.ConflictingFeatures << "\n";
                oss << "    Misc Support: " << detail.FeatureMiscSupport << "\n";
            }
            oss << "\n";
        }
        free(FeatureCaps3D.pFeatureDetails);
    }
    free(hDevices);

    std::string text = oss.str();
    if (text.empty()) text = "No 3D feature capabilities found.";

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
