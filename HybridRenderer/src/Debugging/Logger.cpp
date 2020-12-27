#include "pch.h"
#include "Debugging/Logger.hpp"


#include "Helpers/GeneralHelpers.hpp"
#include "Helpers/magic_enum.hpp"

void Logger::OutputLog() noexcept
{
    m_LogList.remove_if([](const LogEntry& entry) { return entry.markedForClear; });
    //m_LogList.erase(std::remove_if(m_LogList.begin(), m_LogList.end(), [](const LogEntry& entry) { return entry.markedForClear; }), m_LogList.end());

    if (ImGui::Begin("Log"))
    {
        if (ImGui::BeginCombo("LevelSelection", ENUM_TO_C_STR(m_CurrentLevel)))
        {
            for (auto [level, name] : magic_enum::enum_entries<LogLevel>())
            {
                if (ImGui::Selectable(C_STR_FROM_VIEW(name)))
                {
                    m_CurrentLevel = level;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();

        ImGui::Checkbox("Show Headers", &m_ShowHeaders);

        uint32_t logLine = 1;
        for (auto& log : m_LogList)
        {
            if (log.level == m_CurrentLevel || m_CurrentLevel == LogLevel::LEVEL_FULL)
            {
                if (ImGui::SmallButton((std::to_string(logLine++) + "::").c_str()))
                    log.markedForClear = true;

                ImGui::SameLine();

                if (m_ShowHeaders)
                    ImGui::TextColored(m_ImGuiColors.at(magic_enum::enum_integer(log.level)), ("[" + std::string(magic_enum::enum_name(log.level)) + "] " + log.header + " > " + log.message.str()).c_str(), 0);
                else
                    ImGui::TextColored(m_ImGuiColors.at(magic_enum::enum_integer(log.level)), log.message.str().c_str(), 0);
            }
        }
    }
    ImGui::End();
}
