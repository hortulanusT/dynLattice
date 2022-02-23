# :information_source:ParaViewModule 

This Repository contains a JEM/JIVE Module, that enables outputting the results of a calculation to ParaView in XML-Format

In order to include this first execute the command
```
git submodule add git@gitlab.tudelft.nl:tgartner/paraviewmodule.git <desired folder>
```
whereas the desired folder needs to be empty. After this make sure, the Module is found by the JEM/JIVE compile enviroment, thus you need to add in your makefile the line
```
subdirs     += <desired folder>
MY_INCDIRS  := $(subdirs)
```

## :heavy_exclamation_mark:Requirements:heavy_exclamation_mark:
- `utils/testing.h`
- `utils/helpers.h`