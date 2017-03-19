## Server Data Protocol
All data is sent in three parts in a key-value fasion
```
< int value of key from 0-15 >
< options >
< value >
```

Example of sending over the color for what has been set as data set 4 with no options
```
4
0
red
```

#### Options
* `c` - checks value and returns it instead of broadcasting it 
