project "Engine"
   kind "WindowedApp" -- WindowedApp -> SharedLib
   language "C++"
   cppdialect "C++17"
   targetdir "Bin/%{cfg.buildcfg}"
   staticruntime "off"
   location "Engine"
   
   -- NuGet 패키지 참조
   nuget { "Microsoft.Direct3D.D3D12:1.616.1" }

   -- PCH
   pchheader "Pch.h"
   pchsource "Engine/Pch/Pch.cpp" 

   files { "Engine/**.h", "Engine/**.cpp", "Engine/**.hlsl", "Resource/**.rc", "Resource/**" }

   excludes { "Engine/Shaders/**" }

   includedirs
   {
      "Engine",
      "Engine/D3D_Util",
      "Engine/Util",
      "Engine/Renderer",
      "Engine/DirectXTex",
      "Engine/Pch",
      "Engine/Game",
      "Resource",
      "../Vendor"
   }

   vpaths 
   {
      ["Resource/*"] = { "Resource/**" }
   }

   targetdir ("../Bin/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Bin/Intermediate/" .. OutputDir .. "/%{prj.name}")

   filter "files:Engine/DirectXTex/DDSTextureLoader12.cpp"
      flags "NoPCH"

   filter "system:windows"
       systemversion "latest"
        local pkgroot = os.getenv("USERPROFILE") .. "/.nuget/packages/microsoft.direct3d.d3d12/1.616.1/build/native"
        includedirs { pkgroot .. "/include" }
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