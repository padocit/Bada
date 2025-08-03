project "App"
   kind "WindowedApp"
   language "C++"
   cppdialect "C++17"
   targetdir "Bin/%{cfg.buildcfg}"
   staticruntime "off"
   location "App"

   files { "App/**.h", "App/**.cpp", 
   "Resource/**.rc", "Resource/**.h", "../Common/**"}

   includedirs
   {
    "App",
    "App/Utils",
    "Resource",
    "../Common",
    "../Common/Utils"
   }

    -- BadaDX12.lib 가 있는 경로
   libdirs {"../Bin/" .. OutputDir .. "/%{prj.name}"}
   links
   {
      "BadaDX12"
   }

   targetdir ("../Bin/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Bin/Intermediate/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { "WINDOWS" }

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
