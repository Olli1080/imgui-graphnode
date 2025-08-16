#pragma once

#ifndef IMGUI_GRAPHNODE_H_
#define IMGUI_GRAPHNODE_H_

#include <string>

#include "imgui.h"

#ifndef IMGUI_GRAPHNODE_NAMESPACE
#define IMGUI_GRAPHNODE_NAMESPACE ImGuiGraphNode
#endif /* !IMGUI_GRAPHNODE_NAMESPACE */

typedef int ImGuiGraphNodeLayout;

enum ImGuiGraphNodeLayout_
{
    ImGuiGraphNodeLayout_Circo,
    ImGuiGraphNodeLayout_Dot,
    ImGuiGraphNodeLayout_Fdp,
    ImGuiGraphNodeLayout_Neato,
    ImGuiGraphNodeLayout_Osage,
    ImGuiGraphNodeLayout_Sfdp,
    ImGuiGraphNodeLayout_Twopi
};

namespace IMGUI_GRAPHNODE_NAMESPACE
{
    IMGUI_API void CreateContext();
    IMGUI_API void DestroyContext();
    IMGUI_API bool BeginNodeGraph(std::string const& id, ImGuiGraphNodeLayout layout = ImGuiGraphNodeLayout_Dot, float pixel_per_unit = 100.f);
    IMGUI_API void NodeGraphAddNode(std::string const& id);
    IMGUI_API void NodeGraphAddNode(std::string const& id, ImColor const& color, ImColor const& fillcolor);
    IMGUI_API void NodeGraphAddEdge(std::string const& id, std::string const& node_id_a, std::string const& node_id_b);
    IMGUI_API void NodeGraphAddEdge(std::string const& id, std::string const& node_id_a, std::string const& node_id_b, ImVec4 const & color);
    IMGUI_API void EndNodeGraph();
    IMGUI_API const std::string& ImGuiGraphNode_GetEngineNameFromLayoutEnum(ImGuiGraphNodeLayout layout);
}

#endif /* !IMGUI_GRAPHNODE_H_ */
