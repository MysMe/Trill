#pragma once
#include <string_view>

namespace UI
{
	static constexpr std::string_view helpRml =
		R"(
<rml>
	<head>
		<title>Help</title>
		<style>
			body {
				margin: auto;
				font-family: LatoLatin;
			}
			p {
				margin: 0.5em;
				display: block;
			}
			kbd {
				background-color: #f7f7f7;
				border-radius: 3px;
				color: #333;
				display: inline-block;
				font-size: 0.85em;
				line-height: 1.4;
				padding: 2px 4px;
				white-space: nowrap;
			}
		</style>
	</head>
	<body>
		<p>Press <kbd>F1</kbd> to toggle this help screen.</p>
		<p>Press <kbd>Ctrl</kbd> + <kbd>0</kbd> to reset the DPI to the system default.</p>
		<p>Press <kbd>Ctrl</kbd> + <kbd>1</kbd> to set the DPI to 1.</p>
		<p>Press <kbd>Ctrl</kbd> + <kbd>2</kbd> or <kbd>Ctrl</kbd> + <kbd>-</kbd> to decrease the DPI.</p>
		<p>Press <kbd>Ctrl</kbd> + <kbd>3</kbd> or <kbd>Ctrl</kbd> + <kbd>+</kbd> to increase the DPI.</p>
		<p>Press <kbd>Ctrl</kbd> + <kbd>R</kbd> to reload the stylesheets.</p>
		<p>Press <kbd>F8</kbd> to toggle the debugger.</p>
		<p>Use the arrow keys to navigate the UI. Press <kbd>Ctrl</kbd> for precision movements.</p>
		<p>Hold <kbd>Alt</kbd> and use the arrow keys to resize the window.</p>
		<p>Press <kbd>TAB</kbd> to toggle the background.</p>
		<p>Press <kbd>Ctrl</kbd> + <kbd>R</kbd> to reset all settings.</p>
		<p>Press <kbd>Ctrl</kbd> + <kbd>O</kbd> to increase the window opacity.</p>
		<p>Press <kbd>Alt</kbd> + <kbd>O</kbd> to decrease the window opacity.</p>
		<p>Press <kbd>F5</kbd> to force a data fetch.</p>
		<p>Press <kbd>F9</kbd> to refresh the current layout.</p>
	</body>
</rml>
)";
}