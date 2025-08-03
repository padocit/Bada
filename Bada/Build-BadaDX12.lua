project "BadaDX12"
   kind "SharedLib" -- WindowedApp -> SharedLib
   language "C++"
   cppdialect "C++17"
   targetdir "Bin/%{cfg.buildcfg}"
   staticruntime "off"
   location "BadaDX12"
   
   -- NuGet 패키지 참조
   nuget { "Microsoft.Direct3D.D3D12:1.616.1" }

   -- PCH
   pchheader "Pch.h"
   pchsource "BadaDX12/Pch/Pch.cpp" 

   files { 
    "BadaDX12/**.h", "BadaDX12/**.cpp", "BadaDX12/**.hlsl", 
    "Resource/**.rc", "Resource/**", "BadaDX12/**.def",
    "../Common/**", }

   excludes { "BadaDX12/Shaders/**" }

   includedirs
   {
      "BadaDX12",
      "BadaDX12/Utils",
      "BadaDX12/D3D_Util",
      "BadaDX12/Renderer",
      "BadaDX12/DirectXTex",
      "BadaDX12/Pch",
      "BadaDX12/Game",
      "Resource",
      "../Vendor",
      "../Common",
      "../Common/Utils"
   }

   vpaths 
   {
      ["Resource/*"] = { "Resource/**" }
   }

   targetdir ("../Bin/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Bin/Intermediate/" .. OutputDir .. "/%{prj.name}")

   filter "files:BadaDX12/DirectXTex/DDSTextureLoader12.cpp"
      flags "NoPCH"

   filter "system:windows"
       systemversion "latest"
        local pkgroot = os.getenv("USERPROFILE") .. "/.nuget/packages/microsoft.direct3d.d3d12/1.616.1/build/native"
        includedirs { pkgroot .. "/include" }
       defines { }

       postbuildcommands {
      -- (1) App 폴더 없으면 생성
      '{MKDIR} "%{wks.location}/Bin/%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}/App"',
      -- (2) BadaDX12.dll 복사 (파일명까지 명시)
      '{COPYFILE} "%{cfg.buildtarget.abspath}" "%{wks.location}/Bin/%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}/App/BadaDX12.dll"',
      -- (3) BadaDX12.lib 복사
      '{COPYFILE} "%{cfg.linktarget.abspath}" "%{wks.location}/Bin/%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}/App/BadaDX12.lib"',
      }

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

   filter {}
