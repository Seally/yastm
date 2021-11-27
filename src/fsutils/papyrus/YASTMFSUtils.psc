scriptname YASTMFSUtils hidden


; ==============================================================================
; General Filesystem Functions
; ==============================================================================

; Checks if the file at <SkyrimPath>/Data/<filePath> exists.
bool function FileExists(string filePath) global native

; Removes the file at <SkyrimPath>/Data/<filePath>.
bool function RemoveFile(string filePath) global native

; ==============================================================================
; Configuration File Handling
; ==============================================================================

; YASTMFSUtils use TOML as the configuration file format. It operates on
; configuration handles, which is basically an identifier for a configuration
; instance.
;
; '0' indicates a handle creation failure. When opening or creating a
; configuration, check the return value against zero before doing anything else
; with it.
;
; Valid handle values are positive integers, but the exact value should be
; treated as a black box.
;
; After successfully creating a handle*, you MUST call CloseConfig(handle) when
; you're done, otherwise the configuration instance will be kept in memory
; indefinitely.
;
; *There should be no need to call CloseConfig(handle) if handle == 0.
;
;
; EXAMPLE #1:
;
; ; Opens file at "Data/YASTMUserConfig.toml"
; int handle = YASTMFSUtils.OpenConfig("YASTMUserConfig.toml")
;
; if handle != 0
;     ; 0 is the value to return if it's missing. Due to Papyrus type
;     ; restrictions we can't return null.
;     int myValue0 = YASTMFSUtils.LoadInt(handle, "MyValue0", 0)
;     float myValue1 = YASTMFSUtils.LoadFloat(handle, "MyValue1", 0)
;
;     YASTMFSUtils.CloseConfig(handle)
;     ; ... do something with the values
; endIf
;
;
; EXAMPLE #2:
;
; int handle = YASTMFSUtils.CreateConfig()
;
; if handle != 0
;     YASTMFSUtils.SaveInt(handle, "MyValue0", 20)
;     YASTMFSUtils.SaveFloat(handle, "MyValue1", 30)
;
;     ; Saves to "Data/YASTMUserConfig.toml"
;     YASTMFSUtils.SaveConfig(handle, "YASTMUserConfig.toml")
;     YASTMFSUtils.CloseConfig(handle)
; endIf


; Opens the TOML configuration file at <SkyrimPath>/Data/<filePath>.
;
; RETURNS: the configuration handle (0 if failure)
int function OpenConfig(string filePath) global native

; Creates a configuration handle. This is not tied to any file until you call
; SaveConfig() on it.
;
; RETURNS: the configuration handle (0 if failure)
int function CreateConfig() global native

; Writes the contents of the configuration in the given handle to the specified
; path. Like OpenConfig(), the path is resolved against Skyrim's Data directory.
;
; RETURNS: Whether or not the save was successful.
bool function SaveConfig(int configHandle, string filePath) global native

; Closes the handle. This MUST be always be called after OpenConfig() or
; CreateConfig(), except when handle == 0.
function CloseConfig(int configHandle) global native

; Saves an int 'value' to the 'handle' associated with 'key'.
bool function SaveInt(int configHandle, string key, int value) global native

; Saves a float 'value' to the 'handle' associated with 'key'.
bool function SaveFloat(int configHandle, string key, float value) global native

; Loads an int value associated with 'key' in the given 'handle', or the
; 'defaultValue' if the value is missing or has an incompatible type.
int function LoadInt(int configHandle, string key, int defaultValue) global native

; Loads a float value associated with 'key' in the given 'handle', or the
; 'defaultValue' if the value is missing or has an incompatible type.
float function LoadFloat(int configHandle, string key, float defaultValue) global native

; ==============================================================================
; For debugging purposes. Do NOT use in production code!
; ==============================================================================

; Returns the number of open configurations at the current point in time.
int function GetConfigCount() global native

; Returns the largest handle number in the internal map.
int function GetLargestHandle() global native

; Returns the next handle number you will get from the next OpenConfig() or
; CreateConfig() (which may not necessarily be yours).
int function GetNextHandle() global native

; [DANGEROUS] Closes all configuration handles.
;
; NEVER call this function in production code since it will close handles other
; scripts expect to be still be open.
function CloseAllConfigs() global native
