#include <dirent.h>
#include <glob.h>
#include <signal.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <sys/sysctl.h>

#include "file_parser.h"
#include "inotify.h"
#include "rbt.h"


#define NAME "autochown"
#include "version.h"
#define VERSION_FORMAT "%Y-%m-%d %H:%M:%S"
#define VERSION_FORMAT_LENGTH 20


/*!
  @brief
  Verbose mode.
*/
int verbose_mode = 0;

/*!
  @brief
  Dry run.
*/
int dry_run = 0;

/*!
  @brief
  Enable killmask.
*/
int enable_killmask = 0;

/*!
  @brief
  Do not cross devices when recursively scanning directories.
*/
int no_device_crossing = 0;

/*!
  @brief
  Convert a Unix timestamp to a version string.
*/
void
format_version(time_t timestamp, char * version)
{
  struct tm * time;
  time = gmtime(&timestamp);
  strftime(version, VERSION_FORMAT_LENGTH, VERSION_FORMAT, time);
  version[VERSION_FORMAT_LENGTH] = '\0';
}


/*!
  @brief
  Append a trailing slash to a path if it lacks one.

  @param
  path The path.

  @return
  The length of the string.
*/
int
maybe_append_slash(char * path)
{
  int i = strlen(path);
  if (path[i-1] != '/')
  {
    path[i] = '/';
    path[i+1] = '\0';
    i++;
  }
  return i;
}



/*
  @brief
  Chown and chmod a file as necessary.

  @param
  path The target path.

  @param
  st A loaded stat struct for the path, or NULL.

  @param
  target The target struct with information about ownership and mode settings.

  @return
  True if the path no longer exists.
*/
int
adjust_attrib(
  char * path,
  struct stat * st,
  target_t * target
)
{
  const char * filetype;
  char pw_name[MAX_PW_NAME+1];
  char gr_name[MAX_GR_NAME+1];
  int check_mode;
  mode_t mode, mask;
  uid_t uid;
  gid_t gid;
  struct passwd * pw;
  struct group * gr;
  struct stat internal_st;

  pw = NULL;
  gr = NULL;
  check_mode = 0;

  if (st == NULL)
  {
    st = &internal_st;
    if (lstat(path, st))
    {
      if (errno == ENOENT)
      {
        return 1;
      }
      die("error: failed to stat \"%s\" for attribute adjustment", path);
    }
  }


  if (target->chown_uid || target->chown_gid)
  {
    if (target->chown_uid)
    {
      uid = target->uid;
    }
    else
    {
      uid = st->st_uid;
    }

    if (target->chown_gid)
    {
      gid = target->gid;
    }
    else
    {
      gid = st->st_gid;
    }

    if (uid != st->st_uid || gid != st->st_gid)
    {
      if (verbose_mode)
      {

        pw = getpwuid(st->st_uid);
        if (pw != NULL)
        {
          strncpy(pw_name, pw->pw_name, MAX_PW_NAME);
          pw_name[MAX_PW_NAME] = '\0';
          pw = getpwuid(uid);
        }

        gr = getgrgid(st->st_gid);
        if (gr != NULL)
        {
          strncpy(gr_name, gr->gr_name, MAX_GR_NAME);
          gr_name[MAX_GR_NAME] = '\0';
          gr = getgrgid(gid);
        }

        if (pw != NULL && gr != NULL)
        {
          msg_log("lchown %s:%s %s [%s:%s]", pw->pw_name, gr->gr_name, path, pw_name, gr_name);
        }
        else
        {
          msg_log("lchown %lu:%lu %s [%lu:%lu]", uid, gid, path, st->st_uid, st->st_gid);
        }
      }
      if (!dry_run)
      {
        if (lchown(path, uid, gid))
        {
          if (errno == ENOENT)
          {
            return 1;
          }
          if (pw == NULL)
          {
            pw = getpwuid(uid);
          }
          if (gr == NULL)
          {
            gr = getgrgid(gid);
          }
          if (pw != NULL && gr != NULL)
          {
            die("error: failed to change ownership of \"%s\" to %s:%s", path, pw->pw_name, gr->gr_name);
          }
          else
          {
            die("error: failed to change ownership of \"%s\" to %lu:%lu", path, uid, gid);
          }
        }
      }
    }
  }

  /*
    Skip symlinks due to the lack of an "lchmod" function. If they point to
    files in the watched hierarchy then the target file will be updated as
    expected. If the link points to a file outside of the watched hierarchy, it
    should not be touched anyway.

    TODO
    Periodically check for inclusion of lchmod in the standard library.
  */
  if (S_ISLNK(st->st_mode))
  {
    filetype = "symbolic link";
    if (target->chmod_l)
    {
      mask = target->mask_l;
      check_mode = ((mask == KILLMASK) && enable_killmask);
    }
    if (! check_mode)
    {
      if (verbose_mode > 1)
      {
        msg_log("ignoring \"%s\" [%s]", path, filetype);
      }
    }
  }
  else
  {
    switch (st->st_mode & S_IFMT)
    {
      case S_IFDIR:
        mask = target->mask_d;
        check_mode = target->chmod_d;
        filetype = "directory";
        break;
      case S_IFCHR:
        mask = target->mask_c;
        check_mode = target->chmod_c;
        filetype = "character special device file";
        break;
      case S_IFBLK:
        mask = target->mask_b;
        check_mode = target->chmod_b;
        filetype = "block special device file";
        break;
      case S_IFREG:
        mask = target->mask_r;
        check_mode = target->chmod_r;
        filetype = "regular file";
        break;
      case S_IFIFO:
        mask = target->mask_f;
        check_mode = target->chmod_f;
        filetype = "FIFO";
        break;
      case S_IFLNK:
//         mask = target->mask_l;
//         check_mode = target->chmod_l;
//         filetype = "symbolic link";
        die("error: faulty internal logic in handling symlinks (contact the developer)\n");
        break;
      case S_IFSOCK:
        mask = target->mask_s;
        check_mode = target->chmod_s;
        filetype = "socket";
        break;
      default:
        mask = target->mask;
        check_mode = target->chmod;
        filetype = "unrecognized filetype";
        break;
    }
    if (!check_mode && target->chmod)
    {
      mask = target->mask;
      check_mode = target->chmod;
    }
  }

  if (check_mode)
  {
    if ((mask == KILLMASK) && enable_killmask)
    {
      if (strcmp(path, target->target) != 0)
      {
        if (verbose_mode)
        {
          msg_log("removing \"%s\" [%s]", path, filetype);
        }
        if (!dry_run)
        {
          if (remove(path))
          {
            if (errno == ENOENT)
            {
              return 1;
            }
            if (errno != ENOTEMPTY)
            {
              die("error: failed to remove \"%s\"", path);
            }
            else if (verbose_mode)
            {
              msg_log("skipping non-empty directory \"%s\"", path);
            }
          }
          else
          {
            return 1;
          }
        }
      }
    }
    else
    {
      /*
        Replace the group and other modes with the user mode.
        TODO

      */
      mode = st->st_mode & (S_IRWXU | S_IFMT);
      /*
      This was originally done using the following, but the current
      implementation follows the recommendation of not using explicit numbers.

      The supplied masks are admittedly fixed, but the user is responsible for
      those and can adapt them to the system.
      */
//       mode = ((mode >> 3) | (mode >> 6)) | (st->st_mode & ~ (S_IRWXG | S_IRWXO));

      if (mode & S_IRUSR)
      {
        mode |= S_IRGRP | S_IROTH;
      }
      if (mode & S_IWUSR)
      {
        mode |= S_IWGRP | S_IWOTH;
      }
      if (mode & S_IXUSR)
      {
        mode |= S_IXGRP | S_IXOTH;
      }

      mode &= ~(mask);
      if (st->st_mode != mode)
      {
        if (verbose_mode)
        {
          msg_log("chmod %03o %s [%s, %03o]", mode, path, filetype, st->st_mode);
        }
        if (!dry_run)
        {
          if (chmod(path, mode))
          {
            if (errno == ENOENT)
            {
              return 1;
            }
            die("error: failed to change mode of \"%s\" to %03o", path, mode);
          }
        }
      }
    }
  }
  return 0;
}




/*!
  @brief
  Error function to pass to `glob`.

  @param
  filename The filename that caused the error.

  @param
  error The error number (errno).

  @return
  0 to allow glob to continue.
*/
int
glob_errfunc(const char * filename, int error)
{
  msg_log("warning: failed to open \"%s\" [%s]", filename, strerror(errno));
  return 0;
}



/*!
  @brief
  Recursively scan a directory, modifying attributes and building the watchlist.

  @param
  path The path.

  @param
  target The target struct.

  @param
  wd_dict The dictionary to populate with the watch descriptors.

  @param
  watch If "true" then found files and directories will be watched.

  @param
  dev The parent device. This is used to determine device crossing during
  recursion.
*/
void
scan(
  char * path,
  target_t * target,
  wd_node_t * wd_dict,
  int watch,
  dev_t dev
)
{
  char tmp_path[PATH_MAX + 1];
  int l, wd;
  DIR * dir;
  struct dirent * de;
  struct stat st;
  watchlist_data_t data;

  if (verbose_mode > 1)
  {
    msg_log("scanning %s", path);
  }

  if (match_pattern_queue(target->pattern, path) != INCLUDE)
  {
    return;
  }

  if (lstat(path, &st))
  {
    if (errno == ENOENT)
    {
      return;
    }
    die("error: failed to stat \"%s\"", path);
  }


  if (
    adjust_attrib(path, &st, target) ||
    ! S_ISDIR(st.st_mode) ||
    (no_device_crossing && dev && st.st_dev != dev)
  )
  {
    return;
  }

  strcpy(tmp_path, path);
  l = maybe_append_slash(tmp_path);

  if (watch)
  {
    wd = inotify_add_watch(INOTIFY_INSTANCE, path, EVENTS);
    if (wd == -1)
    {
      die("error: failed to add watch (%s)", path);
    }


    data.target = target;
    data.path = tmp_path;

    if (data.path == NULL)
    {
      die("error: failed to duplicate string");
    }

    wd_insert(wd_dict, wd, data);
  }

  dir = opendir(path);
  if (dir == NULL)
  {
    if (errno == ENOENT)
    {
      return;
    }
    die("error: failed to open directory \"%s\"", path);
  }

  errno = 0;
  while ((de = readdir(dir)) != NULL && errno == 0)
  {
    // skip "." and ".."
    if
    (
      de->d_name[0] == '.' &&
      (
        de->d_name[1] == '\0' ||
        (
          de->d_name[1] == '.' &&
          de->d_name[2] == '\0'
        )
      )
    )
    {
      continue;
    }
    strcpy(tmp_path+l, de->d_name);
    scan(tmp_path, target, wd_dict, watch, st.st_dev);
  }
  closedir(dir);
}


/*!
  @brief
  Chown and chmod files and directories (recursively) and optionally watch them
  for further changes.

  @param
  target The target struct.

  @param
  wd_dict The dictionary to populate with the watch descriptors.

  @param
  watch If "true" then found files and directories will be watched.
*/
void
glob_scan(
  target_t * target,
  wd_node_t * wd_dict,
  int watch
)
{
  int i;
  glob_t globbed;

  if (
    glob(
      target->target,
      GLOB_TILDE | GLOB_NOMAGIC,
      glob_errfunc,
      &globbed
    )
  )
  {
    die("error: globbing of \"%s\" failed", target->target);
  }

  for (i=0; i<globbed.gl_pathc; i++)
  {
    scan(globbed.gl_pathv[i], target, wd_dict, watch, 0);
  }
  globfree(&globbed);
}






/*!
  @brief
  Rabbit tree node traversal function to remove watches.
*/
int
remove_all_watches(
  wd_key_data_t * key_data,
  wd_key_size_t height,
  va_list args
)
{
  inotify_rm_watch(INOTIFY_INSTANCE, (int) (* key_data->key));
  return 0;
}





/*!
  @brief
  Print the usage message to a file descriptor.

  @param
  fd The output file descriptor.
*/
void
print_usage(FILE * fd)
{
  char version[VERSION_FORMAT_LENGTH];
  format_version(VERSION, version);
  fprintf(
    fd,
"%s - automatically change file ownership and permissions\n"
"\n"
"version %s\n"
"\n"
"usage:\n"
"  %s [options] <input file>\n"
"\n"
"options:\n"
"  -d: daemonize process\n"
"  -e: update file attributes and exit\n"
"  -k: enable the killmask (%03o)\n"
"  -n: dry run\n"
"  -h: display this message and exit\n"
"  -p: <path>: write PID to path\n"
"  -v: verbose mode (pass multiple times to increase verbosity)\n"
"  -x: disable device crossing when recursing directories\n"
"\n"
"Read the man page for more information.\n"
, NAME, version, NAME, KILLMASK
  );
}



int
main(int argc, char * * argv)
{
  int i, j, l, daemonize, update_and_exit;
  char * pid_path, tmp_path[PATH_MAX + 1], queue_buffer[BUF_LEN];
  struct inotify_event * event;
  FILE * f;
  pid_t pid;
  target_t * targets;
  wd_node_t * wd_dict;
  watchlist_data_t data;

  update_and_exit = 0;
  daemonize = 0;
  pid_path = NULL;

  while((i = getopt(argc, argv, "dehknp:vx")) != -1)
  {
    switch(i)
    {
      case 'h':
        print_usage(stdout);
        return(EXIT_SUCCESS);
        break;
      case 'd':
        daemonize = 1;
        break;
      case 'e':
        update_and_exit = 1;
        break;
      case 'k':
        enable_killmask = 1;
        break;
      case 'n':
        dry_run = 1;
        break;
      case 'p':
        pid_path = optarg;
        break;
      case 'v':
        verbose_mode += 1;
        break;
      case 'x':
        no_device_crossing = 1;
        break;
      default:
        print_usage(stderr);
        return(EXIT_FAILURE);
        break;
    }
  }

  if (argc - optind < 1)
  {
    print_usage(stderr);
    return EXIT_FAILURE;
  }


  if (daemonize)
  {
    msg_log("forking");
    pid = fork();
  }
  else
  {
    pid = getpid();
  }

  if (pid)
  {
    if (! update_and_exit)
    {
      msg_log("pid: %d", pid);
    }
    if (pid_path != NULL)
    {
      if (strcmp(pid_path, "-") == 0)
      {
        printf("%d", pid);
      }
      else
      {
        f = fopen(pid_path, "wb");
        if (f == NULL)
        {
          die("error: failed to open PID file \"%s\"", pid_path);
        }
        fprintf(f, "%d", pid);
        fclose(f);
      }
    }

    if (daemonize)
    {
      /*
        Exit parent.
      */
      exit(EXIT_SUCCESS);
    }
  }




  targets = parse_targets(argv[optind]);

  if (update_and_exit)
  {
    for (i=0; targets[i].target != NULL; i++)
    {
      glob_scan(&targets[i], wd_dict, 0);
    }
    free_targets(targets);
    exit(EXIT_SUCCESS);
  }

  wd_dict = wd_node_new();
  INOTIFY_INSTANCE = inotify_init();

  for (i=0; targets[i].target != NULL; i++)
  {
    glob_scan(&targets[i], wd_dict, 1);
  }



  /*
    This is deprecated. Keeping it here as a reminder.
  */
//   int imqe_path[] = {CTL_FS, FS_INOTIFY, INOTIFY_MAX_QUEUED_EVENTS};
//   int imqe_len = 3;
//   int imqe;
//   imqe = 0;
//   size_t imqe_size = sizeof(imqe);
//   if (sysctl(imqe_path, imqe_len, &imqe, &imqe_size, NULL, 0))
//   {
//     die("error: unable to determine maximum length of event queue");
//   }



//   imqe = read_int("/proc/sys/fs/inotify/max_queued_events");
//   event_queue = calloc(imqe, sizeof(struct inotify_event));
//
//   if (event_queue == NULL)
//   {
//     die("error: failed to allocate memory for event queue");
//   }



  void cleanup(int signal)
  {
    close(INOTIFY_INSTANCE);
//     wd_node_traverse_with_key(wd_dict, remove_all_watches);
    wd_node_free(wd_dict);
    free_targets(targets);
    exit(EXIT_SUCCESS);
  }


  signal(SIGINT, cleanup);


  while ((l = read(INOTIFY_INSTANCE, queue_buffer, BUF_LEN)))
  {
    i = 0;
    while (i < l)
    {
//       e = event_queue[i];
//       i ++;
      event = (struct inotify_event *) &queue_buffer[i];
      i += EVENT_SIZE + event->len;


      /*
        Triggered for items in watched directories: event->name is set
      */
      if (event->mask & (IN_CREATE | IN_MOVED_TO))
      {
        data = wd_retrieve(wd_dict, event->wd);
        strcpy(tmp_path, data.path);
        strcpy(tmp_path + strlen(tmp_path), event->name);
        scan(tmp_path, data.target, wd_dict, 1, 0);
      }


      else if (event->mask & IN_ATTRIB)
      {
        data = wd_retrieve(wd_dict, event->wd);
        strcpy(tmp_path, data.path);
        if (event->len)
        {
          strncat(tmp_path, event->name, event->len);
        }
        scan(tmp_path, data.target, wd_dict, 1, 0);
      }


      /*
        Rescan parent directories when contents are removed to see if a killmask
        should be applied.
      */
      else if (event->mask & IN_DELETE)
      {
        data = wd_retrieve(wd_dict, event->wd);
        strcpy(tmp_path, data.path);
        j = 0;
        while ((tmp_path[j] = data.path[j]) != '\0')
        {
          j ++;
        }
        j--;
        if (tmp_path[j] == '/');
        {
          tmp_path[j] = '\0';
        }
        scan(tmp_path, data.target, wd_dict, 1, 0);
      }

      /*
         Remove directories that get moved. No information is provided about
         the new location, which may be outside of the user-specified paths.
         If the directory was moved to another location within the watched
         hierarchy then it will be caught by IN_MOVED_TO above and re-added.
      */
      else if (event->mask & (IN_DELETE_SELF | IN_MOVE_SELF))
      {
        wd_delete(wd_dict, event->wd);
      }


      /*
        Kill the watchlist and start over if the queue overflows.
      */
      if (event->mask & IN_Q_OVERFLOW)
      {
        wd_node_traverse_with_key(wd_dict, remove_all_watches);
        wd_node_free(wd_dict);
        wd_dict = wd_node_new();

        for (i=0; targets[i].target != NULL; i++)
        {
          glob_scan(&targets[i], wd_dict, 1);
        }
      }
    }
  }

  cleanup(0);

  return EXIT_SUCCESS;
}
