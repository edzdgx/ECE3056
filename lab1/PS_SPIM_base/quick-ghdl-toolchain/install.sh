#!/bin/bash
tar -xf gtkwave.app.tar
printf "Please enter your GTKWave install directory (default ~/Applications/): "
GTKINST="~/Applications"
read INPUT
if [[ ! -z $INPUT ]]
then
    GTKINST=$INPUT
fi
eval GTKINST=$GTKINST
echo "Installing GTKWave to $GTKINST/"
cp -r gtkwave.app "$GTKINST/gtkwave.app"
if [ $? -ne 0 ]
then
    echo "Need to use sudo to install GTKWave"
    echo "You may need to enter your password"
    sudo cp -r gtkwave.app "$GTKINST/gtkwave.app"
fi

echo "\n"
tar -xf GHDL.tar
printf "Please enter the location GHDL should be installed: (default ~/.bin):"
GHDLINST="~/.bin"
read INPUT
if [[ ! -z $INPUT ]]
then
    GHDLINST=$INPUT
fi
eval GHDLINST=$GHDLINST
echo "Installing GHDL to $GHDLINST\n\n"
mkdir -p "$GHDLINST/GHDL"
cp -r GHDL "$GHDLINST/GHDL/"
printf "If this is your first time running this script, you will need to add the installation folder to your PATH\n"
printf "Would you like it to be added to your path automatically? (y/n):"
PATHINST="export PATH=\$PATH:$GHDLINST/GHDL/GHDL/bin"
echo "\n"
read INPUT
case "$INPUT" in
    [yY][eE][sS]|[yY])
        echo $PATHINST >> $HOME/.bash_profile
        echo "Please either restart your terminal or run 'source ~/.bash_profile'"
        ;;
    *)
        echo "Ok, make sure the following line occurs somewhere in ~/.bash_profile"

        echo "$PATHINST"
        ;;
esac


