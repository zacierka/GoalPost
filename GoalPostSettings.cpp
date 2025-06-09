#include "pch.h"
#include "GoalPost.h"

/**
 * GoalPost
 * version: X.X.X-XXX
 * endpoint: |_______| [ping]
 *
*/
void GoalPost::RenderSettings() {
    std::string version = "version: " + std::string(plugin_version);
    ImGui::TextUnformatted(version.c_str());

    // ---- begin enable checkbox ----
    CVarWrapper enableCvar = cvarManager->getCvar("goalpost_enabled");
    if (!enableCvar) { return; }
    bool enabled = enableCvar.getBoolValue();
    if (ImGui::Checkbox("Enable plugin", &enabled)) {
        enableCvar.setValue(enabled);
    }
    // ---- end enable checkbox ----
    
    // ---- begin endpoint textfield/button ----
    CVarWrapper endpointCvar = cvarManager->getCvar("goalpost_endpoint");
    if (!endpointCvar) { return; }
    std::string endpoint = endpointCvar.getStringValue();

    char text[64] = "";
    strncpy(text, endpoint.c_str(), sizeof(text));
    text[sizeof(text) - 1] = '\0';
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
    if (ImGui::InputText("Endpoint", text, IM_ARRAYSIZE(text), flags)) {
        endpointCvar.setValue(std::string(text));
    }


}