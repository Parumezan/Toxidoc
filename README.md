
```
 ______   ______     __  __     __     _____     ______     ______    
/\__  _\ /\  __ \   /\_\_\_\   /\ \   /\  __-.  /\  __ \   /\  ___\   
\/_/\ \/ \ \ \/\ \  \/_/\_\/_  \ \ \  \ \ \/\ \ \ \ \/\ \  \ \ \____  
   \ \_\  \ \_____\   /\_\/\_\  \ \_\  \ \____-  \ \_____\  \ \_____\ 
    \/_/   \/_____/   \/_/\/_/   \/_/   \/____/   \/_____/   \/_____/ 
                                                                      
C++ Documentation Manager
Usage:
  Toxidoc [OPTION...]

  -c, --config arg             Path to config file
  -n, --no-save                Do not save config file after initialization
  -s, --source-paths arg       Source paths (comma separated)
  -r, --recursive              Recursively search directories (default: 
                               true)
  -g, --generate               Generate beginning documentation blocks for 
                               undocumented objects
  -o, --get-object arg         Shows objects whose name matches the 
                               argument you provide
  -x, --header-extensions arg  Header file extensions (comma separated) 
                               (default: .h,.hpp,.hh,.hxx,.ipp,.tpp,.inl)
  -e, --exclude-dirs arg       Directories to exclude (comma separated) 
                               (default: build,.git,third_party,external)
  -b, --blacklist arg          Words to designate names to ignore (comma 
                               separated) (default: Q_PROPERTY)
  -t, --types arg              Blacklist of object types to document (comma 
                               separated) (default: "")
      --type-list              List of available object types
  -v, --verbose                Verbose *LITE* output mode
      --last-update            Show the last update time of the 
                               documentation
  -d, --coverage               Remove the progress bar for documentation 
                               coverage
      --mod arg                add module name for clang parsing (e.g. 
                               --mod path/to/modules/qt_override.h in this 
                               case we use a header to override QT macros, 
                               refers to mods folder to list all modules ; 
                               don't create your own module, the code is 
                               not ready for that)
  -h, --help                   Print usage
```