# audio_thingy
 livecoding audio in c
 currently only on linux, although you only need to change the dynamic library reloading and use gcc equivalents to use it on other operating systems

## interaction
 you can talk to a running audio_thingy program either via the command line or UDP, the commands are the same.
 - **sketch FILENAME.c**: this will tell audio_thingy to use client/sketches/FILENAME.c as the file containing the user functions. look at sketches/default.c for an example of what your file should contain
 - **reload**: this will reload the current sketch. i.e, it will recompile the dynamic library and switch
 - **build**: this will rebuild the current sketch. i.e, it will just recompile, no switching
 - **quit**: quits

## editor integration
 to send messages from your editor to audio_thingy, all you need to do is set a hotkey to execute the "run_command" script
 
### vim
 in vim you can use these functions (assuming you put audio_thingy inside of ~/git/audio_thingy)
 ```vim
 function AudioThingyReload()
     :w
     silent exec "!bash ~/git/audio_thingy/run_command -r"
 endfunction
 
 function AudioThingySketch()
    silent exec "!bash ~/git/audio_thingy/run_command -f"expand('%:t')
 endfunction
 
 " maps audio_thingy reload to <leader> r
 function AudioThingyHotkeys()
     noremap <silent> <leader>r :call AudioThingyReload()<CR>
     inoremap <silent> <leader>r <C-o>:call AudioThingyReload()<CR> 
 endfunction
 
 " sets hotkeys whenever a .c file is opened (not ideal!)
 autocmd BufNewFile,BufRead *.c call AudioThingyHotkeys()
 ```
