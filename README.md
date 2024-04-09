Simplified READ-ONLY FAT32 class for fast accessing and reading of (in my case WAV) files. 
Cluster chains are to be pre-cached (on the application side) for directory files, and stored in memory as a collection of linear sector chain boundaries. Ideally, when no fragmentation appears, only a single pair of sectors (beginning/end) per file needed.

calls are like this:

```
SDMMC_FAT32 Card;
SDMMC_FileReader Reader(&Card);

entry_t* entry;

Card.begin();

Card.testReadSpeed(READ_BUF_SECTORS /*sectors per read*/, 8 /*total MBytes to test*/);

// List root folder
Card.setCurrentDir("/");
Card.printCurrentDir();

entry = Card.nextEntry();
while (!entry->is_end) {
    if (entry->is_dir) {
      Card.setCurrentDir(entry->name);
      Card.printCurrentDir();
      break;
    }
    entry = Card.nextEntry();
}

str_max_t str="";
Reader.open("config.ini"); 
while (Reader.available()) {
    Reader.read_line(str);
    DEBUG(str.c_str());
}
Reader.close();

Card.end();
```

It is neither a product nor a code of beauty, but some stuff one can find useful for particular needs. For example, this may help with reading files from the SD with a reasonable good speed and constant low access time.
For example, standard Arduino ESP32 core lib gives ~5 ms lag before opening first file in the given directory, and ~20 ms lag for the 200th file, because of scanning directory entries.
This class allows addressing files directly by sectors, and reading of sector-aligned blocks.

This is what I have measured for an old random 16GB Card that I had.
|sectors per read  |  reading speed, MB/s |
|------------------|----------------------|
|        1         |        0.90          |
|        2         |        1.57          |
|        4         |        2.82          |
|        8         |        5.00          |
|       16         |        7.79          |
|       32         |       11.12          |
|       64         |       13.85          |
|      128         |       15.75          |
