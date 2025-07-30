#include "imgui_graphnode.h"
#include "imgui_graphnode_internal.h"
#include "imgui_internal.h"

#include <algorithm>
#include <ranges>

namespace internal
{
    ImGuiGraphNodeContext g_ctx;

    ImGuiGraphNode_ShortString<32> ImGuiIDToString(std::string const& id)
    {
        ImGuiGraphNode_ShortString<32> str;

        sprintf_s(str.buf, "%u", ImGui::GetID(id.c_str()));
        return str;
    }

    ImGuiGraphNode_ShortString<16> ImVec4ColorToString(ImVec4 const& color)
    {
        ImGuiGraphNode_ShortString<16> str;
        ImU32 const rgba = ImGui::ColorConvertFloat4ToU32(color);

        sprintf_s(str.buf, "#%x", rgba);
        return str;
    }

    ImU32 ImGuiGraphNode_StringToU32Color(std::string const& color)
    {
        ImU32 rgba;

        sscanf_s(color.c_str(), "#%x", &rgba);
        return rgba;
    }

    ImVec4 ImGuiGraphNode_StringToImVec4Color(std::string const& color)
    {
        return ImGui::ColorConvertU32ToFloat4(ImGuiGraphNode_StringToU32Color(color));
    }

    std::string ImGuiGraphNode_ReadToken(std::ranges::subrange<std::vector<char>::const_iterator>& stringp)
    {
        std::string output;

        char token = stringp.front();
        if (token == '"')
        {
            stringp.advance(1);
            do
            {
                token = stringp.front();
                output += token;
                stringp.advance(1);

                if (token == '\"')
                {
                    stringp.advance(1);
                    break;
                }

            } while (!stringp.empty());

            do
            {
                stringp.advance(1);
                token = stringp.front();

                if (token == ' ')
                {
                    stringp.advance(1);
                    break;
                }
            } while (!stringp.empty());
            return output;
        }
        do
        {
            stringp.advance(1);
            token = stringp.front();

            if (token == ' ')
            {
                stringp.advance(1);
                break;
            }
        } while (!stringp.empty());
        return output;
    }
    /*
    char * ImGuiGraphNode_ReadLine(char ** stringp)
    {
        return strsep(stringp, "\n");
    }
    */
    bool ImGuiGraphNode_ReadGraphFromMemory(ImGuiGraphNodeContextCache& cache, std::vector<char> const& data)
    {
        for (auto line : std::views::split(data, '\n'))
        {
            auto token = ImGuiGraphNode_ReadToken(line);

            if (token == "graph")
            {
                cache.graph.scale = std::stof(ImGuiGraphNode_ReadToken(line));
                cache.graph.size.x = std::stof(ImGuiGraphNode_ReadToken(line));
                cache.graph.size.y = std::stof(ImGuiGraphNode_ReadToken(line));
            }
            else if (token == "node")
            {
                ImGuiGraphNode_Node node;

                node.name = ImGuiGraphNode_ReadToken(line);
                node.pos.x = std::stof(ImGuiGraphNode_ReadToken(line));
                node.pos.y = std::stof(ImGuiGraphNode_ReadToken(line));
                node.size.x = std::stof(ImGuiGraphNode_ReadToken(line));
                node.size.y = std::stof(ImGuiGraphNode_ReadToken(line));
                node.label = ImGuiGraphNode_ReadToken(line);
                ImGuiGraphNode_ReadToken(line); // style
                ImGuiGraphNode_ReadToken(line); // shape
                node.color = ImGuiGraphNode_StringToU32Color(ImGuiGraphNode_ReadToken(line));
                node.fillcolor = ImGuiGraphNode_StringToU32Color(ImGuiGraphNode_ReadToken(line));
                cache.graph.nodes.push_back(node);
            }
            else if (token == "edge")
            {
                ImGuiGraphNode_Edge edge;

                edge.tail = ImGuiGraphNode_ReadToken(line);
                edge.head = ImGuiGraphNode_ReadToken(line);
                const int n = std::stoi(ImGuiGraphNode_ReadToken(line));
                edge.points.resize(n);
                for (int i = 0; i < n; ++i)
                {
                    edge.points[i].x = std::stof(ImGuiGraphNode_ReadToken(line));
                    edge.points[i].y = std::stof(ImGuiGraphNode_ReadToken(line));
                }

                std::string s1 = ImGuiGraphNode_ReadToken(line);
                std::string s2 = ImGuiGraphNode_ReadToken(line);
                std::string s3 = ImGuiGraphNode_ReadToken(line);
                std::string s4 = ImGuiGraphNode_ReadToken(line); (void)s4; // style
                std::string s5 = ImGuiGraphNode_ReadToken(line);
                std::string identifier;

                if (!s3.empty())
                {
                    edge.label = s1;
                    edge.labelPos.x = std::stof(s2);
                    edge.labelPos.y = std::stof(s3);
                    identifier = s5.data() + 1;
                }
                else
                {
                    identifier = s2.data() + 1;
                }

                // Edge ImGuiID is stored in the color property.
                // It is used to access edge info, as graphviz doesn't serialize
                // the edge's identifier. The actual color is then retrieve from
                // the context cache.

                edge.id = std::stoul(identifier, nullptr, 16);
                auto const it = cache.edgeIdToInfo.find(edge.id);
                IM_ASSERT(it != cache.edgeIdToInfo.end());
                edge.color = it->second.color;

                cache.graph.edges.push_back(edge);
            }
            else if (token == "stop")
            {
                return true;
            }
            else
            {
                IM_ASSERT(false);
            }
        }
        return true;
    }

    float ImGuiGraphNode_BSplineVec2ComputeK(std::vector<ImVec2> const& p, std::vector<float> const& t, int i, int k, float x)
    {
        auto const f = [](std::vector<float> const& t, int i, int k, float x) -> float
            {
                if (t[i + k] == t[i])
                    return 0.f;
                return (x - t[i]) / (t[i + k] - t[i]);
            };
        auto g = ImGuiGraphNode_BSplineVec2ComputeK;
        if (k == 0)
            return (x < t[i] || x >= t[i + 1]) ? 0.f : 1.f;
        return f(t, i, k, x) * g(p, t, i, k - 1, x) + (1.f - f(t, i + 1, k, x)) * g(p, t, i + 1, k - 1, x);
    }

    ImVec2 ImGuiGraphNode_BSplineVec2(std::vector<ImVec2> const& p, int m, int n, float x)
    {
        n = std::min(n, m - 2);
        int const knotscount = m + n + 1;

        std::vector<float> knots(knotscount);
        int i = 0;

        for (; i <= n; ++i) knots[i] = 0.f;
        for (; i < m; ++i) knots[i] = i / (float)(m + n);
        for (; i < knotscount; ++i) knots[i] = 1.f;

        ImVec2 result(0.f, 0.f);

        for (i = 0; i < m; ++i)
        {
            float const k = ImGuiGraphNode_BSplineVec2ComputeK(p, knots, i, n, x);

            result.x += p[i].x * k;
            result.y += p[i].y * k;
        }
        return result;
    }

    ImVec2 ImGuiGraphNode_BSplineVec2(std::vector<ImVec2> const& points, int count, float x)
    {
        return ImGuiGraphNode_BSplineVec2(points, count, 3, ImClamp(x, 0.f, 0.9999f));
    }

#define IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_CELL_1(_line, _cell) \
    ImGuiGraphNode_BinomialCoefficient(_line, _cell), \

#define IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_CELL_2(_line, _cell) \
    IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_CELL_1(_line, _cell) \
    IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_CELL_1(_line, _cell + 1)

#define IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_CELL_4(_line, _cell) \
    IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_CELL_2(_line, _cell) \
    IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_CELL_2(_line, _cell + 2)

#define IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_CELL_8(_line, _cell) \
    IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_CELL_4(_line, _cell) \
    IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_CELL_4(_line, _cell + 4)

#define IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_CELL_16(_line, _cell) \
    IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_CELL_8(_line, _cell) \
    IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_CELL_8(_line, _cell + 8)

#define IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_LINE_1(_line) \
    { IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_CELL_16(_line, 0) },

#define IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_LINE_2(_line) \
    IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_LINE_1(_line) \
    IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_LINE_1(_line + 1)

#define IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_LINE_4(_line) \
    IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_LINE_2(_line) \
    IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_LINE_2(_line + 2)

#define IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_LINE_8(_line) \
    IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_LINE_4(_line) \
    IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_LINE_4(_line + 4)

#define IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_LINE_16(_line) \
    IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_LINE_8(_line) \
    IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_LINE_8(_line + 8)

    constexpr int ImGuiGraphNode_BinomialCoefficient(int n, int k)
    {
        return (n == 0 || k == 0 || n == k) ? 1 : ImGuiGraphNode_BinomialCoefficient(n - 1, k - 1) + ImGuiGraphNode_BinomialCoefficient(n - 1, k);
    }

    constexpr int ImGuiGraphNode_BinomialCoefficient_Table[16][16] =
    {
        IMGUIGRAPHNODE_BINOMIALCOEFFICIENT_TABLE_LINE_16(0)
    };

    int ImGuiGraphNode_BinomialCoefficientTable(int n, int k)
    {
        if (n >= 0 && n < 16 && k >= 0 && k < 16)
        {
            return ImGuiGraphNode_BinomialCoefficient_Table[n][k];
        }
        else
        {
            return ImGuiGraphNode_BinomialCoefficient(n, k);
        }
    }

    ImVec2 ImGuiGraphNode_BezierVec2(std::vector<ImVec2> const& points, float x)
    {
        ImVec2 result(0.f, 0.f);

        size_t size = points.size();
        for (size_t i = 0; i < size; ++i)
        {
            float const k = ImGuiGraphNode_BinomialCoefficientTable(size - 1, i) * ImPow(1 - x, size - i - 1) * ImPow(x, i);

            result.x += points[i].x * k;
            result.y += points[i].y * k;
        }
        return result;
    }

    void ImGuiGraphNodeRenderGraphLayout(ImGuiGraphNodeContextCache& cache)
    {
        char* data = nullptr;
        unsigned int size = 0;
        const auto engine = ImGuiGraphNode::ImGuiGraphNode_GetEngineNameFromLayoutEnum(cache.layout);
        int ok = 0;

        cache.graph = ImGuiGraphNode_Graph();
        IM_ASSERT(g_ctx.gvcontext != nullptr);
        IM_ASSERT(g_ctx.gvgraph != nullptr);
        agattr(g_ctx.gvgraph, AGEDGE, (char*)"dir", "none");
        ok = gvLayout(g_ctx.gvcontext, g_ctx.gvgraph, engine.data());
        IM_ASSERT(ok == 0);
        ok = gvRenderData(g_ctx.gvcontext, g_ctx.gvgraph, "plain", &data, &size);
        IM_ASSERT(ok == 0);
        std::vector<char> data_vec(data, data + size);

        ImGuiGraphNode_ReadGraphFromMemory(cache, data_vec);
        gvFreeRenderData(data);
        gvFreeLayout(g_ctx.gvcontext, g_ctx.gvgraph);
    }
}