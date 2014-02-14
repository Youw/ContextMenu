ContextMenu
===========

Simple COM dll that creates new item in explorer context menu named "Make log of file(s) info".

This menu creates or append text file named SelectedFilesInfo.log in the current directory and writes to it for each file:
+last modification date
+per-byte sum (4 bytes variable)
+file size
+short file name

Per-byte sum for files calculating with ThreadPool named JobsExecuter wich can execute specific jobs in different threads. (See JobsExecuting.h)

