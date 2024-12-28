# simple file share with basic C
- If file is exists, server will return the file. ( Yep, that all )

## file format setting
### header file:
- There should be a space(0x20) between 'Content-Length:' and where you want program to insert the value.
- And the file should be ended with 0x0a aka one more line without any text.
- Like this:
```
Content-Length: <the value would be insert here>


```
