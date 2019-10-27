em++ --bind -s STACK_OVERFLOW_CHECK=1 -s SAFE_HEAP=1 -s ASSERTIONS=1 -s EXIT_RUNTIME=1 -s ALLOW_MEMORY_GROWTH=1 -s WASM=0 -s DISABLE_EXCEPTION_CATCHING=0 -s DEMANGLE_SUPPORT=1 -s "EXPORTED_FUNCTIONS=['_javascriptWrapperFunction']" -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap','stringToUTF8', 'lengthBytesUTF8', 'Pointer_stringify']" -std=c++11 -O0 -I midifile/include/ -I src/music_objects/ -I src/data_translation/ src/music_objects/bar.cpp src/music_objects/chunk.cpp src/music_objects/note.cpp src/data_translation/rotate_visitor.cpp src/data_translation/print_visitor.cpp src/data_translation/midi2melody.cpp src/main.cpp  -o src/js_tabber.js
#em++ --bind -s WASM=0 -s "EXPORTED_FUNCTIONS=['_main','_javascriptWrapperFunction']" -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'stringToUTF8', 'lengthBytesUTF8']" -std=c++11 -O0 hello_world.cpp  -o src/js_tabber.js

echo src/js_tabber.js ready for export
#cp src/js_tabber.js ~/Programs/ianbacus.github.com/js_tabber.js
