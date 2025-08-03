-- premake5.lua
workspace "Bada"
   platforms { "x64", "x86" }
   configurations { "Debug", "Release", "Dist" }
   startproject "App"

   filter "platforms:x64"
      architecture "x86_64"

   filter "platforms:x86"
      architecture "x86"

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

   filter {} -- 필터 초기화

OutputDir = "%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

group "Bada"
	include "Bada/Build-BadaDX12.lua"
group ""

include "App/Build-App.lua"