---
documentclass: physycomen
title:  "ubx_to_json"
author: "Fabbri, Sinigardi"
---

<a href="http://www.physycom.unibo.it"> 
<div class="image">
<img src="https://cdn.rawgit.com/physycom/templates/697b327d/logo_unibo.png" width="90" height="90" alt="© Physics of Complex Systems Laboratory - Physics and Astronomy Department - University of Bologna"> 
</div>
</a>
<a href="https://travis-ci.org/physycom/ubx_to_json"> 
<div class="image">
<img src="https://travis-ci.org/physycom/ubx_to_json.svg?branch=master" width="90" height="20" alt="Build Status"> 
</div>
</a>
<a href="https://ci.appveyor.com/project/cenit/ubx-to-json"> 
<div class="image">
<img src="https://ci.appveyor.com/api/projects/status/1q8veq5gg35xsfwl?svg=true" width="90" height="20" alt="Build Status"> 
</div>
</a>

### Purpose
This tool has been written to convert data from the u-blox binary proprietary format (.ubx) to our .json structure, in order to be able to use data coming from the u-Center software inside our tools.

### Installation
**make** and a **C++11** compatible compiler are required. Clone the repo and type ``make``, it should be enough in most cases!   
There's also a **VS2015** solution avalaible.   
Contains [jsoncons](https://github.com/danielaparker/jsoncons) as a git submodule.

### Usage
```
ubx_to_json.exe -i input.ubx -o output.json
```
where `input.ubx` must be an existing and valid .ubx file, while `output.json` is the name of the output file.

More details about file formats is available [here](https://github.com/physycom/file_format_specifications/blob/master/formati_file.md).

