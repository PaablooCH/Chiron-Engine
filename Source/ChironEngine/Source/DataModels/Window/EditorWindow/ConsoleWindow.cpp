#include "Pch.h"
#include "ConsoleWindow.h"

namespace
{
    const std::vector<size_t> consoleLineLengths{ 10U, 25U, 50U, 100U };
    std::optional<size_t> selectedMaxLinesIndex = 3U; // default to 50 lines max
} // namespace

ConsoleWindow::ConsoleWindow() : EditorWindow(ICON_FA_TERMINAL " Console", ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar)
{
    _severityFilters[LogSeverity::INFO_LOG] = true;
    _severityFilters[LogSeverity::TRACE_LOG] = true;
    _severityFilters[LogSeverity::DEBUG_LOG] = true;
    _severityFilters[LogSeverity::WARNING_LOG] = true;
    _severityFilters[LogSeverity::ERROR_LOG] = true;
    _severityFilters[LogSeverity::FATAL_LOG] = true;

    _severityColors[LogSeverity::INFO_LOG] = ImVec4(1.f, 1.f, 1.f, 1.f);        // white
    _severityColors[LogSeverity::TRACE_LOG] = ImVec4(0.f, 1.f, 1.f, 1.f);       // cyan
    _severityColors[LogSeverity::DEBUG_LOG] = ImVec4(0.22f, 1.f, 0.08f, 1.f);   // lime
    _severityColors[LogSeverity::WARNING_LOG] = ImVec4(1.f, 1.f, 0.f, 1.f);     // yellow
    _severityColors[LogSeverity::ERROR_LOG] = ImVec4(1.f, 0.f, 0.f, 1.f);       // red
    _severityColors[LogSeverity::FATAL_LOG] = ImVec4(1.f, 0.f, 1.f, 1.f);       // purple
}

ConsoleWindow::~ConsoleWindow()
{
}

void ConsoleWindow::DrawWindowContent(const std::shared_ptr<CommandList>& commandList)
{
    _consoleContents.insert(std::end(_consoleContents), std::begin(logContext->_logLines), std::end(logContext->_logLines));
    logContext->_logLines.clear();

    DrawOptionsMenu();
    DrawConsole();
}

void ConsoleWindow::DrawOptionsMenu()
{
    if (ImGui::BeginMenuBar())
    {
        CHIRON_TODO("don't use a menu(?)");
        if (ImGui::BeginMenu("Filters"))
        {
            for (auto& filter : _severityFilters)
            {
                const char* text = "";
                switch (filter.first)
                {
                case LogSeverity::INFO_LOG:
                    text = "INFO";
                    break;
                case LogSeverity::TRACE_LOG:
                    text = "TRACE";
                    break;
                case LogSeverity::DEBUG_LOG:
                    text = "DEBUG";
                    break;
                case LogSeverity::WARNING_LOG:
                    text = "WARNING";
                    break;
                case LogSeverity::ERROR_LOG:
                    text = "ERROR";
                    break;
                case LogSeverity::FATAL_LOG:
                    text = "FATAL";
                    break;
                }
                ImGui::MenuItem(text, nullptr, &filter.second);
            }
            ImGui::EndMenu();
        }

        if (ImGui::Button("Clear"))
        {
            _consoleContents.clear();
        }

        DrawMaxLengthSelection();

        ImGui::EndMenuBar();
    }
}

void ConsoleWindow::DrawMaxLengthSelection()
{
    int numButtons = static_cast<int>(consoleLineLengths.size());
    const char* limitText = "Limit console lines:";
    const char* noLimitButtonText = "No limit";

    // Align the buttons to the right
    ImGuiStyle& style = ImGui::GetStyle();

    float buttonWidth = ImGui::CalcTextSize("XXX").x + style.FramePadding.x * 2.0f;
    float buttonContainerWidth = (buttonWidth + style.ItemSpacing.x) * numButtons;

    float noLimitButtonWidth = ImGui::CalcTextSize(noLimitButtonText).x + style.FramePadding.x * 2.0f;
    float noLimitButtonContainerWidth = noLimitButtonWidth + style.ItemSpacing.x;

    float textWidth = ImGui::CalcTextSize(limitText).x + style.FramePadding.x * 2.0f;

    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - noLimitButtonContainerWidth - buttonContainerWidth - textWidth);

    ImGui::TextUnformatted(limitText);
    ImGui::SameLine();

    for (int i = 0; i < numButtons; ++i)
    {
        ImGui::PushID(i);

        // store the condition in case it changes during draw
        bool selected = selectedMaxLinesIndex.has_value() && i == selectedMaxLinesIndex.value();
        if (selected)
        {
            ImVec4 activeColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
            ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
        }

        if (ImGui::Button(std::to_string(consoleLineLengths[i]).c_str(), ImVec2(buttonWidth, 0)))
        {
            selectedMaxLinesIndex = i;
        }

        if (selected)
        {
            ImGui::PopStyleColor();
        }

        ImGui::PopID();

        ImGui::SameLine();
    }

    // store the condition in case it changes during draw
    bool selected = !selectedMaxLinesIndex.has_value();

    if (selected)
    {
        ImVec4 activeColor = ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive);
        ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
    }

    if (ImGui::Button(noLimitButtonText, ImVec2(noLimitButtonWidth, 0)))
    {
        selectedMaxLinesIndex = std::nullopt;
    }

    if (selected)
    {
        ImGui::PopStyleColor();
    }
}

void ConsoleWindow::DrawConsole()
{
    size_t linesToDraw =
        selectedMaxLinesIndex.has_value() ? consoleLineLengths[selectedMaxLinesIndex.value()] : _consoleContents.size();
    auto linesFiltered = _consoleContents |
        std::views::filter(
            [this](const Chiron::Log::LogLine& logLine)
            {
                return _severityFilters[logLine.severity];
            }) |
        // Reverse to take the last N elements, then reverse again to maintain line order
                std::views::reverse | std::views::take(linesToDraw) | std::views::reverse;

            // Define the position from which text will start to wrap, which will be the end of the console window
            ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);

            for (const Chiron::Log::LogLine& logLine : linesFiltered)
            {
                ImGui::TextColored(_severityColors[logLine.severity], logLine.ToSimpleString().c_str());
                // Add spacing between each line
                ImGui::Dummy(ImVec2(0.0f, 0.5f));
            }

            ImGui::PopTextWrapPos();

            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            {
                ImGui::SetScrollHereY(1.0f);
            }
}