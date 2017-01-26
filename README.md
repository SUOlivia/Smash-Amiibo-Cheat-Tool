# Smash Amiibo Cheat Tool
Hack your amiibos in a tap.

Based off the libctru NFC example

## Menu
Hack: Simple and quick way to buff your amiibo's stats to 200 everywhere

Restore Backup / Bruteforce appdata writing: Restore a backup if previously backed up by the `Hack` function or the `Only dump appdata` function / Writes a modified appdata to the bruteforced amiibo

Only dump appdata / Bruteforce appdata dump: Only dumps a backup file / Dumps the appdate of the bruteforced amiibo

Custom file writing: Writes `/Smash amiibo cheat tool/write.amiibo` to the amiibo

Appdata Randomizing: Randomizes the appdata of your amiibo (bruteforcing supported), this can be used for grinding rewards in smash for example 

## Tagstates
1: Stopped scanning for Amiibos/NFC tags

2: Currently scanning for Amiibos/NFC tags

3: An Amiibo/NFC tag has been found

4: Usually happens after tagstate 3, Amiibo/NFC tag out of range

5: The Amiibo's/NFC tag's data successfully loaded

## Retuned values
0xC8A17628: The amiibo haven't been setup yet

0xC8A17638: Smash data haven't been found

0xC8C1760C and 0xC8A17618: The amiibo is corrupted

0x00000000: Everything's fine

## Credits
Yellows8 : Documentation on smash amiibo data

MarcusD: Helping me with some code issues

Thunder Kai: All the graphics

Yudowat, CloudedSun, Fug/leni, jasgx, Karma, LinkSoraZelda, Swiftloke and Y2K: Testing and ideas

## IT IS IMPOSSIBLE TO MODIFY THE AMIIBO'S IDENTITY