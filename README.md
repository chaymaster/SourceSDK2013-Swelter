# SourceSDK2013-Swelter
Tea is the best thing in our life.

## Building
1. Install Microsoft Visual Studio 2013. You can find it [here](https://visualstudio.microsoft.com/en/vs/older-downloads/).
2. Clone the repository: `git clone https://github.com/chaymaster/SourceSDK2013-Swelter.git`
3. Specify path to your mod's `bin` folder in `game/client/client_base.vpc` AND `game/server/server_base.vpc`:
    * e.g. `$Macro OUTBINDIR	"C:\Steam\steamapps\common\Swelter\Swelter\bin"`
4. Generate the solution by running `creategameprojects.bat`.
5. Open the solution using VS2013 or your preferrable version of Visual Studio.
6. Change the build configuration to `Release`.
7. Build `Client (Episodic)` and `Server (Episodic)` projects.
    * Output dlls are automatically placed to your output directory specified at Step 3.
8. Done! You can launch the mod and test it.
