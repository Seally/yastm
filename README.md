# YASTM SKSE Plugin

This plugin implements the SKSE portion of the YASTM (Yet Another Soul Gem
Manager) mod for Skyrim Special Edition.

It fixes the crash that occurs when using a reusable soul gem whose base form 
already has a soul. In order for reusable soul gems to be supported by this 
plugin, all non-empty reusable soul gems must have their NAM0 field (linked soul 
gem) filled with their empty version.

## Requirements
* [CMake](https://cmake.org/)
* [Vcpkg](https://github.com/microsoft/vcpkg)
	* Add the environment variable `VCPKG_ROOT` with the value as the path to the
	  folder containing vcpkg
* [Visual Studio Community 2019](https://visualstudio.microsoft.com/)
	* Desktop development with C++
	* The project currently does not officially support any compilers other than 
	  MSVC. Everything is written with the assumption that MSVC is the compiler, 
	  especially because it is also the compiler used for Skyrim SE.

## Build Instructions

1. Clone the project _and_ its submodules using 
   `git clone --recurse-submodules <repo_url>`.
2. Open the project in Visual Studio (preferred version: Visual Studio 2019).
2. Set CMake variables:
   * SKYRIM64_DATA_PATH - Set this to the path of Skyrim's Data folder. 
     Tip: You can also set this to the mod folder you're working on if you're
	 using something like Mod Organizer 2.
3. Build the project.
