#include "UI.h"
#include "ProjectDir.h"
#include <filesystem>

#include <iostream>
#include <RmlUi/Core.h>
#include <RmlUi_Backend.h>
#include <RmlUi/Debugger.h>

#include "Request.h"
#include "APIData.h"
#include "Settings.h"
#include <GLFW/glfw3.h>

#include "Keybinds.h"
#include <format>
#include "UIModel.h"

#include "Troncbox.h"

#ifdef _WIN32
#define NOMINMAX
#define GLFW_EXPOSE_NATIVE_WIN32
#undef APIENTRY
#include <Windows.h>
#include <GLFW/glfw3native.h>
#endif

namespace UI
{
	static settings::settings userSettings;
	static Rml::ElementDocument* helpDocument = nullptr;
    static Rml::ElementDocument* contentDocument = nullptr;

    //Struct that can be easily read and reset as a single operation
    struct flag
    {
        bool value = false;
        void set() { value  = true; }
        operator bool()
        {
            bool current = value;
			value = false;
			return current;
        }
    };

    struct
    {
        //Indicates the settings should be updated
        flag settingsDirty;
		//Indicates the window should be updated
        flag windowDirty;
		//Indicates the data should be fetched
        flag forceFetch;
		//Indicates the document should be refreshed
        flag refreshDocument;
    } 
    //Stores UI callback events that can be handled from the main code
    eventResults;

    bool ProcessKeyDownShortcuts(Rml::Context *context, Rml::Input::KeyIdentifier key, int key_modifier, float native_dp_ratio, bool priority)
    {
        if (!context)
            return true;

        // Result should return true to allow the event to propagate to the next handler.
        bool result = false;

        // This function is intended to be called twice by the backend, before and after submitting the key event to the context. This way we can
        // intercept shortcuts that should take priority over the context, and then handle any shortcuts of lower priority if the context did not
        // intercept it.
        if (priority)
        {
            // Priority shortcuts are handled before submitting the key to the context.

			bool ctrlDown = key_modifier & Rml::Input::KM_CTRL;
			bool altDown = key_modifier & Rml::Input::KM_ALT;
			int16_t motion = ctrlDown ? 1 : 10;

            if (key == Rml::Input::KI_F8)
            {
                Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
            }
            else if (key == Rml::Input::KI_0 && ctrlDown)
            {
                context->SetDensityIndependentPixelRatio(native_dp_ratio);
				userSettings.dp_ratio = native_dp_ratio;
				eventResults.settingsDirty.set();
            }
            else if (key == Rml::Input::KI_1 && ctrlDown)
            {
                context->SetDensityIndependentPixelRatio(1.f);
				userSettings.dp_ratio = 1.f;
                eventResults.settingsDirty.set();
            }
            else if ((key == Rml::Input::KI_2 || key == Rml::Input::KI_SUBTRACT) && ctrlDown)
            {
                const float new_dp_ratio = Rml::Math::Max(context->GetDensityIndependentPixelRatio() / 1.2f, 0.5f);
                context->SetDensityIndependentPixelRatio(new_dp_ratio);
				userSettings.dp_ratio = new_dp_ratio;
                eventResults.settingsDirty.set();
            }
            else if ((key == Rml::Input::KI_3 || key == Rml::Input::KI_ADD) && ctrlDown)
            {
                const float new_dp_ratio = Rml::Math::Min(context->GetDensityIndependentPixelRatio() * 1.2f, 2.5f);
                context->SetDensityIndependentPixelRatio(new_dp_ratio);
				userSettings.dp_ratio = new_dp_ratio;
                eventResults.settingsDirty.set();
            }
            else if (key == Rml::Input::KI_R && ctrlDown)
            {
                userSettings = {};
                eventResults.windowDirty.set();
            }
			else if (key == Rml::Input::KI_UP)
			{
                if (altDown)
					userSettings.window_height -= motion;
                else
					userSettings.window_y -= motion;
                eventResults.windowDirty.set();
			}
			else if (key == Rml::Input::KI_DOWN)
			{
				if (altDown)
					userSettings.window_height += motion;
				else
					userSettings.window_y += motion;
                eventResults.windowDirty.set();
			}
			else if (key == Rml::Input::KI_LEFT)
			{
				if (altDown)
					userSettings.window_width -= motion;
				else
					userSettings.window_x -= motion;
                eventResults.windowDirty.set();
			}
			else if (key == Rml::Input::KI_RIGHT)
			{
				if (altDown)
					userSettings.window_width += motion;
				else
					userSettings.window_x += motion;
                eventResults.windowDirty.set();
			}
			else if (key == Rml::Input::KI_0 && ctrlDown && altDown)
			{
				userSettings.opacity = 1.0f;
                eventResults.windowDirty.set();
			}
			else if (key == Rml::Input::KI_O && ctrlDown)
			{
				userSettings.opacity = Rml::Math::Min(userSettings.opacity + 0.1f, 1.0f);
                eventResults.windowDirty.set();
			}
			else if (key == Rml::Input::KI_O && altDown)
			{
				userSettings.opacity = Rml::Math::Max(userSettings.opacity - 0.1f, 0.1f);
                eventResults.windowDirty.set();
			}
            else if (key == Rml::Input::KI_TAB)
            {
				userSettings.transparent = !userSettings.transparent;
                eventResults.settingsDirty.set();
            }
			else if (key == Rml::Input::KI_F5)
			{
                eventResults.forceFetch.set();
			}
            else if (key == Rml::Input::KI_F9)
			{
                eventResults.refreshDocument.set();
			}
            else if (key == Rml::Input::KI_F1)
            {
				if (!helpDocument)
				{
					helpDocument = context->LoadDocumentFromMemory(helpRml.data(), "help.rml");
					if (helpDocument)
					{
                        contentDocument->Hide();
						helpDocument->Show();
					}
				}
				else
				{
					helpDocument->Close();
					helpDocument = nullptr;
                    contentDocument->Show();
				}
            }
            else
            {
                // Propagate the key down event to the context.
                result = true;
            }
        }
        else
        {
            // We arrive here when no priority keys are detected and the key was not consumed by the context. Check for shortcuts of lower priority.
            if (key == Rml::Input::KI_R && key_modifier & Rml::Input::KM_CTRL)
            {
                for (int i = 0; i < context->GetNumDocuments(); i++)
                {
                    Rml::ElementDocument *document = context->GetDocument(i);
                    const Rml::String &src = document->GetSourceURL();
                    if (src.size() > 4 && src.substr(src.size() - 4) == ".rml")
                    {
                        document->ReloadStyleSheet();
                    }
                }
            }
            else
            {
                result = true;
            }
        }

        return result;
    }

	void fetch(model& data, const requestHandler& api, const api::data& apiData, const troncboxHandler& troncbox)
    {
		const auto issues = issues::getCurrentIssues(api, apiData);
		const auto resolutions = resolutions::getTicketsResolvedToday(api, apiData);
        const auto tbResult = troncbox.get();
		data.set(issues, resolutions, tbResult);
    }

	//Use this to ensure the function is called when the scope is exited
    template <class Fn>
    struct atScopeExit
    {
        Fn fn;
        atScopeExit(Fn fn) : fn(fn) {}
        ~atScopeExit() { fn(); }
    };

    void renderToUI(bool allowClick)
    {
		userSettings = settings::getSettings();

        const auto keys = keys::loadKeys(constants::project_path / "keys/");
        const auto api = requestHandler(keys);
        const auto apiData = api::getApiData(api);
        const auto troncbox = troncboxHandler(keys);

        model data;
		//Initial fetch
		fetch(data, api, apiData, troncbox);

        auto windowPtr = Backend::Initialize("Trill", userSettings.window_width, userSettings.window_height, true, allowClick);
		atScopeExit backendCleanUp([&] { Backend::Shutdown(); });

#ifdef _WIN32
        if (!allowClick)
        {
            auto hwnd = glfwGetWin32Window(static_cast<GLFWwindow *>(windowPtr));
            ShowWindow(hwnd, SW_HIDE);
            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
            ShowWindow(hwnd, SW_SHOW);
        }
#endif

        // Constructs the system and render interfaces, creates a window, and attaches the renderer.
        if (!windowPtr)
        {
            return;
        }

		auto window = reinterpret_cast<GLFWwindow*>(windowPtr);

        //Set window position
        glfwSetWindowPos(window, userSettings.window_x, userSettings.window_y);
		glfwSetWindowOpacity(window, userSettings.opacity);

        // Install the custom interfaces constructed by the backend before initializing RmlUi.
        Rml::SetSystemInterface(Backend::GetSystemInterface());
        Rml::SetRenderInterface(Backend::GetRenderInterface());

        // RmlUi initialisation.
        Rml::Initialise();
		atScopeExit rmlCleanUp([] { Rml::Shutdown(); });

        // Create the main RmlUi context.
        Rml::Context *context = Rml::CreateContext("main", Rml::Vector2i(userSettings.window_width, userSettings.window_height));
        if (!context)
        {
            return;
        }

        // The RmlUi debugger is optional but very useful. Try it by pressing 'F8' after starting this sample.
        Rml::Debugger::Initialise(context);

        // Load fonts
        auto resourcesDirectory = constants::project_path / "resources";
        // Iterate all ttfs in resources directory
        for (const auto &entry : std::filesystem::directory_iterator(resourcesDirectory))
        {
            if (entry.path().extension() == ".ttf")
            {
                Rml::LoadFontFace(entry.path().string());
            }
        }

		Rml::DataModelHandle dataModel;
		data.bind(context, dataModel);

		contentDocument = context->LoadDocument((resourcesDirectory / "layout.rml").string());
		if (contentDocument)
        {
            contentDocument->Show();
        }

		std::chrono::time_point<std::chrono::system_clock> lastFetch = std::chrono::system_clock::now();

        context->SetDensityIndependentPixelRatio(userSettings.dp_ratio);

        bool running = true;
        while (running)
        {
            //Handle input and window events.
            running = Backend::ProcessEvents(context, &ProcessKeyDownShortcuts, true);

			//If we need to refresh the document, do so now
			if (eventResults.refreshDocument)
			{
				//Unload and reload the document
                contentDocument->Close();
				contentDocument = context->LoadDocument((resourcesDirectory / "layout.rml").string());
				if (contentDocument)
				{
					contentDocument->Show();
				}
			}

			//If the window is dirty, update the window
            if (eventResults.windowDirty)
            {
                glfwSetWindowPos(window, userSettings.window_x, userSettings.window_y);
				glfwSetWindowSize(window, userSettings.window_width, userSettings.window_height);
				context->SetDimensions(Rml::Vector2i(userSettings.window_width, userSettings.window_height));
				glfwSetWindowOpacity(window, userSettings.opacity);
				//Indicate that the settings are dirty, changing the window may have changed the window settings
                eventResults.settingsDirty.set();
            }

            //Determine what the new window settings would be
			settings::settings newSettings = userSettings;
            int x, y;
			glfwGetWindowPos(window, &x, &y);
			newSettings.window_x = static_cast<decltype(newSettings.window_x)>(x);
			newSettings.window_y = static_cast<decltype(newSettings.window_y)>(y);
			glfwGetWindowSize(window, &x, &y);
			newSettings.window_width = static_cast<decltype(newSettings.window_width)>(x);
			newSettings.window_height = static_cast<decltype(newSettings.window_height)>(y);

			//If the settings are dirty or have changed, save them
            if (newSettings != userSettings || eventResults.settingsDirty)
            {
				settings::saveSettings(newSettings);
				userSettings = newSettings;
            }

			//If we haven't fetched in the last minute, fetch now
            if (eventResults.forceFetch || std::chrono::system_clock::now() - lastFetch > std::chrono::seconds(60))
            {
				fetch(data, api, apiData, troncbox);
				dataModel.DirtyAllVariables();
				lastFetch = std::chrono::system_clock::now();
            }

            // Always update the context before rendering.
            context->Update();

            // Prepare the backend for taking rendering commands from RmlUi and then render the context.
            Backend::BeginFrame(userSettings.transparent ? 0.f : 1.f);
            context->Render();
            Backend::PresentFrame();
        }
    }
}