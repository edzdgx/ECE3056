# quick-ghdl-toolchain
## Status
The installer should now work. If you ran the installer before 2:10 April 10, delete the last line added to your ~/.bash_profile. Then navigate to the local copy of this repository and run

    git pull
   
## Overview
This is designed to help 3056 students quickly complete their vhdl simulation projects without wasting valueable tiem with the shortcoming and slowness of Modelsim. This toolchain is based off of GHDL and GTKWave - both venerable open source projects.
## Benefits
This toolchain takes a total of 1 or 2 seconds to re-compile VHDL code and re-display it - best of all, it preserves the layout of signal files accross sessions - so no need to re add signals upon every compile.
## Support
This toolchain currently only supports MacOS. Canonicals Ubuntu has adopted a very troubling repository policy making it difficult for me to make this toolchain easily accessible to users of debian-derived Linux distros.

In addition, the is no app-package for GHDL - while I could make one in theory - I am not up to speed on the new app-package format and I generally try to stay away from generating static compiles in linux as this can be very daunting - and the kernel changes from release to release - creating possible issues with the dynamic loader.
## Installation
Open up terminal and run these commands:

    git clone https://github.com/BracketMaster/quick-ghdl-toolchain.git
    cd quick-ghdl-toolchain
    make install-OSX
The last command will probably ask for your password when installing gtk-wave - so pay close attention! Next, test that ghdl has been properly installed by typing 

    ghdl
It should respond with: 
    
    ghdl:error: missing command, try ghdl --help
If it doesn't respond, type 

    source ~/.bash_profile 
and try again.

Next, copy the makefile in the "quick-ghdl-toolchain" directory to wherever your vhdl file are, then once you are ready to simulate, navigate to the directory with your vhdl files and type `make` in your terminal.

Finally, use GTKWave to view your waveform - it should be installed under applications on your Mac
## Using GTKWave
1. To view your Waveform in GTKWave - double click on the file that ends in .ghw in your project directory - for this project - it should be "spim_pipe.ghw"
2. You select the signal family from the left in the SST panel, then select the specific signal in "Signals" panel which is below the "SST" panel.
3. Drag the selected signal into the tall "Signals" pane to the right
4. Your signal should now show up on the graph
5. Use the zoom buttons +/- to zoom out from the default femto seconds to nanoseconds
6. To save your worksession so that you can return to view the selected signals, do, file -> write savefile -> save it somewhere. This new file should end in .gtkw. Double click the .gtkw file in finder and your session will be restored.
## Recompiling
Navigate to your vhdl directory in the terminal and type `make`, then switch over to your open waveform in GTKWave and press command+shift+r and your waveform will be reloaded.
