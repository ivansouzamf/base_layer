add_rules("mode.debug", "mode.release")

target("base_layer")
    set_kind("static")
    set_languages("cxx20")
    add_files("src/**.cpp")

    if is_plat("window") then
        add_rules("win.sdk.application")
    end

	if is_mode("debug") then
		add_defines("DEBUG")
	end
target_end()
