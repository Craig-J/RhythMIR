#include "console.h"
#include <Agnostic\logger.h>

namespace sfx
{
	ImGuiConsole::ImGuiConsole(sf::RenderWindow& _window) :
		window_(_window),
		out_(agn::Log::CaptureStreamOutput())
	{
		ClearLog();
		Log("Console initialized.");
	}

	ImGuiConsole::~ImGuiConsole()
	{
		ClearLog();
	}

	void ImGuiConsole::ClearLog()
	{
		items_.clear();
		scroll_to_bottom_ = true;
	}

	void ImGuiConsole::Log(std::string _message)
	{
		LogMsg msg;
		msg.msg = _message;
		if (strstr(msg.msg.c_str(), "fatal error")) msg.type = LogMsg::MSGTYPE::FATAL_ERROR_MSG;
		else if (strstr(msg.msg.c_str(), "error")) msg.type = LogMsg::MSGTYPE::ERROR_MSG;
		else if (strstr(msg.msg.c_str(), "warning")) msg.type = LogMsg::MSGTYPE::WARNING_MSG;
		else if (strstr(msg.msg.c_str(), "important")) msg.type = LogMsg::MSGTYPE::IMPORTANT_MSG;
		else msg.type = LogMsg::MSGTYPE::MSG;
		items_.push_back(msg);
		scroll_to_bottom_ = true;
	}

	void ImGuiConsole::Draw(const char* title, bool* opened)
	{
		UpdateStream();

		ImGui::SetNextWindowPos(ImVec2(220, 10));
		
		if (!ImGui::Begin(title, opened, ImVec2(window_.getSize().x - 230, window_.getSize().y * 0.2f), -1.f,
						  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
		{
			ImGui::End();
			return;
		}

		if (ImGui::SmallButton("Clear")) ClearLog();
		ImGui::SameLine();
		ImGui::TextWrapped("Press F10 to toggle the console and frame measures.");

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		static ImGuiTextFilter filter;
		filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
		ImGui::PopStyleVar();
		ImGui::Separator();

		// Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
		// NB- if you have thousands of entries this approach may be too inefficient. You can seek and display only the lines that are visible - CalcListClipping() is a helper to compute this information.
		// If your items are of variable size you may want to implement code similar to what CalcListClipping() does. Or split your data into fixed height items to allow random-seeking into your list.
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetItemsLineHeightWithSpacing()), false, ImGuiWindowFlags_HorizontalScrollbar);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
		ImGuiListClipper clipper(items_.size(), ImGui::GetTextLineHeight());
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			const char* item = items_[i].msg.c_str();
			if (!filter.PassFilter(item))
				continue;

			ImVec4 colour;
			switch (items_[i].type)
			{
			case LogMsg::MSGTYPE::FATAL_ERROR_MSG:
				colour = ImColor(255, 0, 0);
				break;
			case LogMsg::MSGTYPE::ERROR_MSG:
				colour = ImColor(255, 0, 0);
				break;
			case LogMsg::MSGTYPE::WARNING_MSG:
				colour = ImColor(169, 255, 0);
				break;
			case LogMsg::MSGTYPE::IMPORTANT_MSG:
				colour = ImColor(255, 169, 0);
				break;
			default:
				colour = ImColor(255, 255, 255);
				break;
			}
			ImGui::PushStyleColor(ImGuiCol_Text, colour);
			ImGui::TextWrapped(item);
			ImGui::PopStyleColor();
		}
		if (scroll_to_bottom_)
			ImGui::SetScrollHere();
		scroll_to_bottom_ = false;
		clipper.End();
		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::End();
	}

	void ImGuiConsole::UpdateStream()
	{
		std::string line;
		while (std::getline(out_, line))
		{
			Log(line);
		}
		out_.clear();
	}
}