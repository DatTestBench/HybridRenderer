#ifndef MESH_PARSER_HPP
#define MESH_PARSER_HPP

//Project includes
#include <vector>
#include <fstream>
#include <string>
#include <regex>
#include <map>
#include <functional>
#include <algorithm>
#include <tuple>

#include "Helpers/EMath.h"
class Mesh;

class MeshParser
{
public:
    MeshParser()
        : m_Index{0}
    {
        LoadParseFunctions();
    }

    ~MeshParser() = default;

    std::tuple<bool, std::vector<uint32_t>, std::vector<VertexInput>> ParseMesh(const std::string& fileName)
    {
        std::vector<std::string> vInput;
        std::ifstream input;
        input.open(fileName, std::ios::in | std::ios::binary);
        int idx{};
        if (input.is_open())
        {
            for (std::string line; std::getline(input, line);)
            {
                vInput.push_back(line);
            }
            input.close();
        }

        idx = 0;
        const std::regex tokenCheck("^([^\\s]*)");
        for (auto& i : vInput)
        {
            std::smatch match;
            bool searchMatch = regex_search(i, match, tokenCheck);
            auto search = m_ParseFunctions.find(match[1]);
            if (search != m_ParseFunctions.end() && searchMatch)
                m_ParseFunctions.at(std::string(match[1]))(i);
        }
        MakeTangents();
        return std::make_tuple(true, m_IndexBuffer, m_VertexBuffer);
    }

private:
    std::map<std::string, std::function<void(const std::string&)>> m_ParseFunctions;
    //Output buffers
    std::vector<uint32_t> m_IndexBuffer;
    std::vector<VertexInput> m_VertexBuffer;

    //Working buffers
    std::vector<Elite::FPoint3> m_VertexPosBuffer;
    std::vector<Elite::FVector2> m_UVBuffer;
    std::vector<Elite::FVector3> m_NormalBuffer;

    int m_Index;


    void MakeTangents()
    {
        for (uint64_t i = 0; i < m_IndexBuffer.size(); i += 3)
        {
            auto index0 = m_IndexBuffer[i];
            auto index1 = m_IndexBuffer[i + 1];
            auto index2 = m_IndexBuffer[i + 2];

            const auto& p0 = m_VertexBuffer[index0].pos;
            const auto& p1 = m_VertexBuffer[index1].pos;
            const auto& p2 = m_VertexBuffer[index2].pos;
            const auto& uv0 = m_VertexBuffer[index0].uv;
            const auto& uv1 = m_VertexBuffer[index1].uv;
            const auto& uv2 = m_VertexBuffer[index2].uv;

            const auto edge0 = p1 - p0;
            const auto edge1 = p2 - p0;
            const auto diffX = Elite::FVector2(uv1.x - uv0.x, uv2.x - uv0.x);
            const auto diffY = Elite::FVector2(uv1.y - uv0.y, uv2.y - uv0.y);
            auto r = 1.f / Elite::Cross(diffX, diffY);

            auto tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
            m_VertexBuffer[index0].tangent += tangent;
            m_VertexBuffer[index1].tangent += tangent;
            m_VertexBuffer[index2].tangent += tangent;
        }
        //Create the tangents (reject vector) + fix the tangents per vertex
        for (auto& v : m_VertexBuffer)
            v.tangent = GetNormalized(Reject(v.tangent, v.normal));
    }

    void AddVertex(const Elite::FPoint3& pos, const Elite::FVector2& uv, const Elite::FVector3 normal)
    {
        uint32_t currentIndex{};
        uint32_t findIndex{};
        const auto findResult = std::find_if(m_VertexBuffer.begin(), m_VertexBuffer.end(),
                                             [&, this](const VertexInput& vertex)
                                             {
                                                 currentIndex++;
                                                 if (vertex.pos == pos && vertex.uv == uv && vertex.normal == normal)
                                                 {
                                                     findIndex = currentIndex - 1;
                                                     return true;
                                                 }
                                                 return false;
                                             });
        if (findResult != m_VertexBuffer.end())
        {
            m_IndexBuffer.emplace_back(findIndex);
        }
        else
        {
            m_VertexBuffer.emplace_back(pos, uv, normal);
            m_IndexBuffer.emplace_back(m_Index++);
        }
    }

    void AddVertex(const Elite::FPoint3& pos)
    {
        AddVertex(pos, Elite::FVector2{}, Elite::FVector3{});
    }

    void AddVertex(const Elite::FPoint3& pos, const Elite::FVector2& uv)
    {
        AddVertex(pos, uv, Elite::FVector3{});
    }

    void AddVertex(const Elite::FPoint3& pos, const Elite::FVector3& normal)
    {
        AddVertex(pos, Elite::FVector2{}, normal);
    }

    void LoadParseFunctions()
    {
        m_ParseFunctions["#"] = [this](const std::string&)
        {
            //Exists to handle comments without causing undefined behaviour, does not process the line
            return;
        };
        m_ParseFunctions["v"] = [this](const std::string& string)
        {
            const std::regex regex("([-e\\d.]+) ([-e\\d.]+) ([-e\\d.]+)");
            //Slimmed down regex, may not have 100% format coverage
            std::smatch match;
            regex_search(string, match, regex);
            // Adding - to the z, because DirectX is left handed!
            m_VertexPosBuffer.push_back(Elite::FPoint3{stof(match[1]), stof(match[2]), -stof(match[3])});
        };
        m_ParseFunctions["vt"] = [this](const std::string& string)
        {
            const std::regex regex("([-e\\d.]+) ([-e\\d.]+)"); //Slimmed down regex, may not have 100% format coverage
            std::smatch match;
            regex_search(string, match, regex);

            m_UVBuffer.push_back(Elite::FVector2(stof(match[1]), 1 - stof(match[2])));
        };
        m_ParseFunctions["vn"] = [this](const std::string& string)
        {
            const std::regex regex("([-e\\d.]+) ([-e\\d.]+) ([-e\\d.]+)");
            //Slimmed down regex, may not have 100% format coverage
            std::smatch match;
            regex_search(string, match, regex);

            m_NormalBuffer.push_back(Elite::FVector3(stof(match[1]), stof(match[2]), stof(match[3])));
        };
        m_ParseFunctions["vp"] = [this](const std::string&)
        {
            //Exists to handle vp without causing undefined behaviour, does not process the line
            return;
        };
        m_ParseFunctions["f"] = [this](const std::string& string)
        {
            const std::regex regex("(\\d*)\\/(\\d*)\\/(\\d*) (\\d*)\\/(\\d*)\\/(\\d*) (\\d*)\\/(\\d*)\\/(\\d*)");
            //Slimmed down regex, may not have 100% format coverage
            std::smatch match;
            regex_search(string, match, regex);

            AddVertex(m_VertexPosBuffer[stoll(match[1]) - 1], m_UVBuffer[stoll(match[2]) - 1],
                      m_NormalBuffer[stoll(match[3]) - 1]);
            AddVertex(m_VertexPosBuffer[stoll(match[4]) - 1], m_UVBuffer[stoll(match[5]) - 1],
                      m_NormalBuffer[stoll(match[6]) - 1]);
            AddVertex(m_VertexPosBuffer[stoll(match[7]) - 1], m_UVBuffer[stoll(match[8]) - 1],
                      m_NormalBuffer[stoll(match[9]) - 1]);
        };
    }
};

#endif // !MESH_PARSER_HPP
