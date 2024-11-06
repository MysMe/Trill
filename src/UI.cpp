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

namespace UI
{
	static settings::settings userSettings;
	static bool settingsDirty = false;
	static bool windowDirty = false;
	static bool forceFetch = false;

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
				settingsDirty = true;
            }
            else if (key == Rml::Input::KI_1 && ctrlDown)
            {
                context->SetDensityIndependentPixelRatio(1.f);
				userSettings.dp_ratio = 1.f;
                settingsDirty = true;
            }
            else if ((key == Rml::Input::KI_2 || key == Rml::Input::KI_SUBTRACT) && ctrlDown)
            {
                const float new_dp_ratio = Rml::Math::Max(context->GetDensityIndependentPixelRatio() / 1.2f, 0.5f);
                context->SetDensityIndependentPixelRatio(new_dp_ratio);
				userSettings.dp_ratio = new_dp_ratio;
				settingsDirty = true;
            }
            else if ((key == Rml::Input::KI_3 || key == Rml::Input::KI_ADD) && ctrlDown)
            {
                const float new_dp_ratio = Rml::Math::Min(context->GetDensityIndependentPixelRatio() * 1.2f, 2.5f);
                context->SetDensityIndependentPixelRatio(new_dp_ratio);
				userSettings.dp_ratio = new_dp_ratio;
				settingsDirty = true;
            }
            else if (key == Rml::Input::KI_R && ctrlDown)
            {
                userSettings = {};
				windowDirty = true;
            }
			else if (key == Rml::Input::KI_UP)
			{
                if (altDown)
					userSettings.window_height -= motion;
                else
					userSettings.window_y -= motion;
                windowDirty = true;
			}
			else if (key == Rml::Input::KI_DOWN)
			{
				if (altDown)
					userSettings.window_height += motion;
				else
					userSettings.window_y += motion;
                windowDirty = true;
			}
			else if (key == Rml::Input::KI_LEFT)
			{
				if (altDown)
					userSettings.window_width -= motion;
				else
					userSettings.window_x -= motion;
                windowDirty = true;
			}
			else if (key == Rml::Input::KI_RIGHT)
			{
				if (altDown)
					userSettings.window_width += motion;
				else
					userSettings.window_x += motion;
                windowDirty = true;
			}
			else if (key == Rml::Input::KI_0 && ctrlDown && altDown)
			{
				userSettings.opacity = 1.0f;
				windowDirty = true;
			}
			else if (key == Rml::Input::KI_O && ctrlDown)
			{
				userSettings.opacity = Rml::Math::Min(userSettings.opacity + 0.1f, 1.0f);
				windowDirty = true;
			}
			else if (key == Rml::Input::KI_O && altDown)
			{
				userSettings.opacity = Rml::Math::Max(userSettings.opacity - 0.1f, 0.1f);
				windowDirty = true;
			}
            else if (key == Rml::Input::KI_TAB)
            {
				userSettings.transparent = !userSettings.transparent;
				settingsDirty = true;
            }
			else if (key == Rml::Input::KI_F5)
			{
				forceFetch = true;
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

    struct appData
    {
        size_t hotfixes = 3;
        size_t hotfixesUnseen = 0;
        std::string hotfixesLast;

        size_t medium = 4;
		size_t mediumUnseen = 0;
		std::string mediumLast;

        size_t low = 2;
		size_t lowUnseen = 0;
		std::string lowLast;

        size_t veryLow = 3;
		size_t veryLowUnseen = 0;
		std::string veryLowLast;

        size_t resolved = 1;
        std::string syncLast;
    };

    bool setupDataBinding(Rml::Context* context, Rml::DataModelHandle& model, appData& source)
    {
        Rml::DataModelConstructor constructor = context->CreateDataModel("model");
        if (!constructor)
            return false;

		constructor.Bind("hotfixes", &source.hotfixes);
        constructor.Bind("hotfixesUnseen", &source.hotfixesUnseen);
		constructor.Bind("hotfixesLast", &source.hotfixesLast);

		constructor.Bind("medium", &source.medium);
		constructor.Bind("mediumUnseen", &source.mediumUnseen);
		constructor.Bind("mediumLast", &source.mediumLast);

		constructor.Bind("low", &source.low);
		constructor.Bind("lowUnseen", &source.lowUnseen);
		constructor.Bind("lowLast", &source.lowLast);

		constructor.Bind("veryLow", &source.veryLow);
		constructor.Bind("veryLowUnseen", &source.veryLowUnseen);
		constructor.Bind("veryLowLast", &source.veryLowLast);

		constructor.Bind("resolved", &source.resolved);
        constructor.Bind("syncLast", &source.syncLast);

        model = constructor.GetModelHandle();

        return true;
    }

	void fetch(appData& data, const requestHandler& api, const api::data& apiData)
    {
		const auto issues = issues::getCurrentIssues(api, apiData);
		const auto resolutions = resolutions::getTicketsResolvedToday(api, apiData);

		const auto& hotfix = issues.getOr(fields::priority::hotfix, {});
		data.hotfixes = hotfix.count;
		data.hotfixesUnseen = hotfix.unseen;
		data.hotfixesLast = std::format("{:%F %T}", std::chrono::floor<std::chrono::seconds>(hotfix.lastUpdate));

		const auto& medium = issues.getOr(fields::priority::regular, {});
		data.medium = medium.count;
		data.mediumUnseen = medium.unseen;
		data.mediumLast = std::format("{:%F %T}", std::chrono::floor<std::chrono::seconds>(medium.lastUpdate));

		const auto& low = issues.getOr(fields::priority::low, {});
		data.low = low.count;
		data.lowUnseen = low.unseen;
		data.lowLast = std::format("{:%F %T}", std::chrono::floor<std::chrono::seconds>(low.lastUpdate));

		const auto& veryLow = issues.getOr(fields::priority::veryLow, {});
		data.veryLow = veryLow.count;
		data.veryLowUnseen = veryLow.unseen;
		data.veryLowLast = std::format("{:%F %T}", std::chrono::floor<std::chrono::seconds>(veryLow.lastUpdate));

		data.resolved = resolutions;
		data.syncLast = std::format("{:%F %T}", std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()));
    }

    template <class Fn>
    struct atScopeExit
    {
        Fn fn;
        atScopeExit(Fn fn) : fn(fn) {}
        ~atScopeExit() { fn(); }
    };

    void renderToUI()
    {
		userSettings = settings::getSettings();

        const auto api = requestHandler(keys::loadKeys(constants::project_path / "keys/"));
        const auto apiData = api::getApiData(api);

        appData data;
		fetch(data, api, apiData);

        auto windowPtr = Backend::Initialize("Trill", userSettings.window_width, userSettings.window_height, true);
		atScopeExit backendCleanUp([&] { Backend::Shutdown(); });

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
		setupDataBinding(context, dataModel, data);

		Rml::ElementDocument* document = context->LoadDocument((resourcesDirectory / "layout.rml").string());
		if (document)
        {
            document->Show();
        }

		std::chrono::time_point<std::chrono::system_clock> lastFetch = std::chrono::system_clock::now();

        context->SetDensityIndependentPixelRatio(userSettings.dp_ratio);

        bool running = true;
        while (running)
        {
            // Handle input and window events.
            running = Backend::ProcessEvents(context, &ProcessKeyDownShortcuts, true);

			if (forceFetch)
			{
				fetch(data, api, apiData);
				dataModel.DirtyAllVariables();
				forceFetch = false;
			}

            if (windowDirty)
            {
                glfwSetWindowPos(window, userSettings.window_x, userSettings.window_y);
				glfwSetWindowSize(window, userSettings.window_width, userSettings.window_height);
				context->SetDimensions(Rml::Vector2i(userSettings.window_width, userSettings.window_height));
				glfwSetWindowOpacity(window, userSettings.opacity);
				windowDirty = false;
                settingsDirty = true;
            }

			settings::settings newSettings = userSettings;
            int x, y;
			glfwGetWindowPos(window, &x, &y);
			newSettings.window_x = static_cast<decltype(newSettings.window_x)>(x);
			newSettings.window_y = static_cast<decltype(newSettings.window_y)>(y);
			glfwGetWindowSize(window, &x, &y);
			newSettings.window_width = static_cast<decltype(newSettings.window_width)>(x);
			newSettings.window_height = static_cast<decltype(newSettings.window_height)>(y);

            if (newSettings != userSettings || settingsDirty)
            {
				settings::saveSettings(newSettings);
				userSettings = newSettings;
                settingsDirty = false;
            }

            if (std::chrono::system_clock::now() - lastFetch > std::chrono::seconds(60))
            {
				fetch(data, api, apiData);
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