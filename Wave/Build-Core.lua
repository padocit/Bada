project "Core"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "Bin/%{cfg.buildcfg}"
   staticruntime "off"
   location "Core"

    -- PCH 설정
   pchheader "Pch.h"
   pchsource "Core/Pch.cpp"

   files { "Core/**.h", "Core/**.cpp" }

   includedirs
   {
      "Core"
   }

   targetdir ("../Bin/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Bin/Intermediate/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { }

   filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"

   filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"

   filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"