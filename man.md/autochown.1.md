# Name

autochown - Monitor multiple directories using glob patterns and automatically adjust file ownership and permissions.

# Synopsis

`autochown [options] <input file>`

# Description
`autochown` is a tool for group collaboration. It recursively monitors shared directories with inotify and responds to events that affect file ownership and permissions. It can be used to automatically chown and chmod files to ensure that target files are accessible to certain users but not others.

`autochown` can be used in combination with access control lists and other methods for sharing directories between users.

**Note:** `autochown` must be run with superuser permissions to be able to chown files.


# Input File

## Basics
`autochown`'s behavior is determined by the input file. The input file has a simple yet versatile format.

Lines beginning with '#' or a space are ignored.

Lines the begin with "> " indicate target directories. They have the following format:

    > <user>:<group>:<mask>:<glob pattern>

`<user>`, `<group>` and `<mask>` are all optional and may be omitted, but the colon field delimiters must remain. See the examples below.

`autochown` will recurse into directories that match `<glob pattern>`. If `<user>` is given then it will change the owner of the file to the given user name. If `<group>` is given then it will change the group to the given group name.

The "group" and "other" mode of the file will be set to the "user" mode. If `<mask>` is given then it will be applied to the resulting mode.

For example,

    > nobody:users:007:/tmp/test

will change the owner and group of the directory /tmp/test and all files in it to "nobody" and "users", respectively. The mask `007` will then be applied to each file after copying the "user" mode to the "group" and "other"^[This is only done in an internal mask calculation. The file mode is not updated twice.] To make this clear with an example, consider a file with the mode `755`. Applying the "user" mode to the other two modes gives `777`. The `007` mask is then applied to this mode to give the final mode of `770`. The effect of the `007` mask is to copy the "user" mode to the "group" mode and deny all access to "other".

To change the user to "nobody" but keep the current group and mode, use

    > nobody:::/tmp/test

To change the user to "nobody" and deny access to everyone else, use

    > nobody::077:/tmp/test

Greater control of the target files can be achieved by using inclusion and exclusion patterns:

    + <glob pattern>
    - <glob pattern>

The following will do the same as the previous example, except that all files and directories in /tmp/test that begin with "private-" will be ignored:

    > nobody:users:007:/tmp/test
    - /tmp/test/private-*

This can be further refined with an inclusion line:

    > nobody:users:007:/tmp/test
    - /tmp/test/private-*
    + /tmp/test/private-users

The above adds /tmp/test/private-users to the targets while still excluding all other paths that begin with "/tmp/test/private-".


Multiple targets may be specified in a single file:

    > nobody:users:007:/tmp/test
    - /tmp/test/private

    > nobody::077:/tmp/test/private

The above will make all files and directories in /tmp/test accessible to members of the "users" group, except for /tmp/test/private, which will only be accessible by "nobody".

## Per-filemode Masks
In some cases you may wish to apply different masks to different file types. For example, you may wish to unset the executable bit on all files while leaving it on all directories. This can be done by preceeding the octal mask in the target line with one of the following characters:

D
:   directory

C
:   character special file

B
:   block special file

R
:   regular file

F
:   FIFO special file

L
:   symbolic link

S
:   socket

Starting with the previous example

    > nobody:users:007:/tmp/test

which would apply the mask 007 to all filetypes, we could change it to

    > nobody:users:117D007:/tmp/test

The mask specifier here is `117D007`. This is interpretted as a default mask of `117` and a directory mask of `007`. If a file is a directory then `007` will be applied, otherwise `117` will be applied. This effectively removes the executable bit from all files except directories in the target directory hierarchy.

The order is irrelevant and the mask specifier could just as well be given as

    > nobody:users:D007117:/tmp/test

The only time that order matters is if the same mask type is specified twice, e.g.

    > nobody:users:017D007117:/tmp/test

The default mask in this case will be `117` because it was specified after `017`.

Note that no default mask is required. If none is given then files will only be chmod'd if they match a given filetype mask.


For the exact interpretation of the different file types, consult the "Testing File Type" section of the [GNU C library documentation](http://www.gnu.org/software/libc/manual/html_mono/libc.html#Testing-File-Type). The character specifiers above match the `S_IS...` macros there.



## Killmask
The mask `700` would be non-sensical given the above rules so it is given a special meaning by Autochown. `700` is the killmask. It instructs Autochown to remove matching files. This is very useful to prevent certain filetypes from appearing in the target directory. For example, to prevent the creation of FIFOs, the previous example could be changed to

    > nobody:users:007F700:/tmp/test

Be **very careful** when using the killmask. It can wipe out an entire directory hierarchy if you make a mistake. To minimize the risk of a catastrophic typo, the killmask is only recognized when the `-k` option is passed.


Setting the killmask for directories (`D700`) will *not* recursively remove non-empty directories directly. Automatic removal of files inside a directory via the killmask will triggger inotify events as the directory is emptied so that it may eventually be removed.

Killmasks do not apply to the paths in the target line. It would amount to a Rube Goldberg implementation of `rm` otherwise.
