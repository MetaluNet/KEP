# KEP
**Pd patches set for building musical instruments based on Fraise KEP interfaces**

The KEP standard interface is a Fraise controller containing 5 rotating potentiometers, 4 general purpose keys, 1 shift key and 1 record key, each with an "active" led.  
The present patches allow to define a number of instruments, and to link dynamically an interface to an instrument, using an abstraction layer named CACHE.

## prerequisites
 - Pd 0.46.7 minimum
 - Required externals:
 	- moonlib
	- iemlib
	- ggee
	- zexy
	- maxlib
	- motex
	- hcs
	- creb
	- moocow
	- cxc
	- mrpeach
 - Fraise (in order to use KEP controllers) : follow https://github.com/MetaluNet/Fraise

## installation
Create an empty folder somewhere to host your project.
In this folder, do:

```
git clone --recursive https://github.com/MetaluNet/KEP.git
```

First copy all files and folders located into ```KEP/0USER_TEMPLATE/``` and paste them into your project folder.

Then open the newly copied MasterDemo.pd with Pd.

## customizing

- Fill DATAS/SAMPLES with your favorite wav-files, to be able to open them in the Stretcher(Stre) or the Decouper(Dec).

- Customize KEP_INSTRUMENTS set :

	- remove instruments you don't want.  
	- add instruments you want to have ; the syntax is : [Instr/MAIN InstrN], where Instr is the name of the instrument's folder (e.g Rythmic) and N is an integer number (start from 1).

- Configure audio routing and levels of the instruments, and of MASTERS window, and store the settings in MASTERS.

- Assign every CACHE to an instrument, and save KEPcontrol in CACHEguis window : tap "sel" button on the CACHE, and browse to the setting file of the instrument you want to link to this CACHE : InstrFolder->SETTING->the_setting_file (e.g BASE).

- Edit a new song in SONG/GamSeq. Store the song choice in SONG/KEPsong's patterns to make your playlist, and save to file.

- Edit instruments internal settings, then SAVE_SETTING, then update CACHEs settings to use the newly created settings for this instrument. 




