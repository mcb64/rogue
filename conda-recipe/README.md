
#### Building conda package

````
$ conda build -q conda-recipe --python=3.6 --output-folder bld-dir -c defaults -c conda-forge -c paulscherrerinstitute
$ conda activate
$ anaconda upload bld-dir/linux-64/rogue-.....
````