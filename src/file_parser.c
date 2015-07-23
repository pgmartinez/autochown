#include "file_parser.h"

/*!
  @brief
  Count the number of newlines in a file.

  @param
  fd The file descriptor. It is assumed to be at the beginning of the file and
  will be reset to it when done.
*/
// int
// count_targets(FILE * fd)
// {
//   char buffer[BUFSIZE];
//   int i, n, after_newline;
//
//   after_newline = 1;
//   n = 0;
//   memset(buffer, 0, BUFSIZE);
//
//   while (fread(buffer, 1, BUFSIZE, fd))
//   {
//     for (i=0; i<BUFSIZE; i++)
//     {
//       if (buffer[i] == '\n')
//       {
//         after_newline = 1;
//       }
//       else
//       {
//         if (buffer[i] == '>' && after_newline)
//         {
//           n ++;
//         }
//         after_newline = 0;
//       }
//     }
//   }
//   fseek(fd, 0L, SEEK_SET);
//   return n;
// }



/*!
  @brief
  Read file content into a buffer.

  @param
  fpath The file path.

  @return
  The queue of sequential patterns to match.
*/
target_t *
parse_targets(char * fpath)
{
  int i, j, n, is_stdin, size, initialized, chown_uid, chown_gid;
  char line[(PATH_MAX * 3) / 2];

  FILE * fd;

  action_t action;
  target_t * targets, * tmptargets;
  pattern_t * * next_pattern;

  uid_t uid;
  gid_t gid;
  struct group * gr;
  struct passwd * pw;

  char filetype;
  int chmod, chmod_d, chmod_c, chmod_b, chmod_r, chmod_f, chmod_l, chmod_s;
  mode_t mask, tmp_mask, mask_d, mask_c, mask_b, mask_r, mask_f, mask_l, mask_s;

  if (fpath == NULL)
  {
    die("error: no filepath given");
  }

  is_stdin = (strcmp(fpath, "-") == 0);

  if (is_stdin)
  {
    fd = stdin;
  }
  else
  {
    fd = fopen(fpath, "r");
    if (fd == NULL)
    {
      die("error: failed to open \"%s\"", fpath);
    }
  }

//   n = count_targets(fd);
//   targets = malloc(n * sizeof(target_t));

  size = 10;
  targets = malloc(size * sizeof(target_t));

  if (targets == NULL)
  {
    die("error: failed to allocate memory for input");
  }

  uid = 0;
  gid = 0;
  chown_uid = 0;
  chown_gid = 0;
  mask = 0;
  tmp_mask = 0;
  mask_d = 0;
  mask_c = 0;
  mask_b = 0;
  mask_r = 0;
  mask_f = 0;
  mask_l = 0;
  mask_s = 0;
  chmod = 0;
  chmod_d = 0;
  chmod_c = 0;
  chmod_b = 0;
  chmod_r = 0;
  chmod_f = 0;
  chmod_l = 0;
  chmod_s = 0;
  filetype = '\0';
  /*
    Initialize this to -1 so that it can be incremented before assigning the
    target etc. Incrementing it after would require (n-1) for each associated
    pattern.
  */
  n = -1;
  initialized = 0;
  next_pattern = NULL;

  while(fgets(line, sizeof(line), fd))
  {
    /*
      > user:group:<3-digit octal mask>:<path>
    */
    if (line[0] == '>')
    {
      if (line[1] != ' ')
      {
        die(
          "error: malformed input file: missing space after '>'"
        );
      }
      i = 2;

      /*
        Parse user.
      */
      if (line[i] == ':')
      {
        chown_uid = 0;
        i ++;
      }
      else
      {
        for (j=i; j<PATH_MAX; j++)
        {
          if (line[j] == ':')
          {
            line[j] = '\0';
            pw = getpwnam(line + i);
            if (pw == NULL)
            {
              die(
                "error: failed to determine uid for user %s",
                line + i
              );
            }
            uid = pw->pw_uid;
            chown_uid = 1;
            line[j] = ':';
            break;
          }
          else if (line[j] == '\0' || line[j] == ']')
          {
            die(
              "error: malformed input file: unexpected header termination"
            );
          }
        }
        i = j+1;
      }


      /*
        Parse group.
      */
      if (line[i] == ':')
      {
        chown_gid = 0;
        i ++;
      }
      else
      {
        for (j=i; j<PATH_MAX; j++)
        {
          if (line[j] == ':')
          {
            line[j] = '\0';
            gr = getgrnam(line + i);
            if (gr == NULL)
            {
              die(
                "error: failed to determine gid for group %s",
                line + i
              );
            }
            gid = gr->gr_gid;
            chown_gid = 1;
            line[j] = ':';
            break;
          }
          else if (line[j] == '\0' || line[j] == ']')
          {
            die(
              "error: malformed input file: unexpected header termination"
            );
          }
        }
        i = j+1;
      }



      /*
        Parse masks.
      */
      mask = 0;
      mask_d = 0;
      mask_c = 0;
      mask_b = 0;
      mask_r = 0;
      mask_f = 0;
      mask_l = 0;
      mask_s = 0;
      chmod = 0;
      chmod_d = 0;
      chmod_c = 0;
      chmod_b = 0;
      chmod_r = 0;
      chmod_f = 0;
      chmod_l = 0;
      chmod_s = 0;
      if (line[i] == ':')
      {
        if (chown_uid + chown_gid == 0)
        {
          initialized = 0;
          continue;
        }
        else
        {
          i ++;
        }
      }
      else
      {
        while (1)
        {
          tmp_mask = 0;
          if (line[i] < '0' || line[i] > '7')
          {
            filetype = line[i];
            i ++;
          }
          else
          {
            filetype = '\0';
          }
          for (j=i; j<i+3; j++)
          {
            tmp_mask <<= 3;
            if (line[j] >= '0' && line[j] <= '7')
            {
              tmp_mask += (line[j] - '0');
            }
            else
            {
              die(
                "error: malformed input file: non-octal character in mask (%c)",
                line[j]
              );
            }
          }
          switch (filetype)
          {
            case '\0':
              chmod = 1;
              mask = tmp_mask;
              break;

            case 'D':
              chmod_d = 1;
              mask_d = tmp_mask;
              break;

            case 'C':
              chmod_c = 1;
              mask_c = tmp_mask;
              break;

            case 'B':
              chmod_b = 1;
              mask_b = tmp_mask;
              break;

            case 'R':
              chmod_r = 1;
              mask_r = tmp_mask;
              break;

            case 'F':
              chmod_f = 1;
              mask_f = tmp_mask;
              break;

            case 'L':
              chmod_l = 1;
              mask_l = tmp_mask;
              break;

            case 'S':
              chmod_s = 1;
              mask_s = tmp_mask;
              break;

            default:
//               errno = ENOTSUP;
              errno = EINVAL;
              die(
                "error: malformed input file: unrecognized file type specifier (%c)",
                filetype
              );
          }
          i = j;
          if (line[j] == ':')
          {
            i ++;
            break;
          }
          else if (line[j] == '\0')
          {
            die(
              "error: malformed input file: unexpected character in header"
            );
          }
        }
      }

      /*
        Add path.
      */

      for (j=i; line[j] != '\0'; j++)
      {
        if (line[j] == '\n')
        {
          line[j] = '\0';
          break;
        }
      }
      if (line[j-1] == '/')
      {
        line[j-1] = '\0';
      }

      n ++;
      if (n == size)
      {
        size *= 2;
        if (size <= n)
        {
          size = INT_MAX;
          if (size <= n)
          {
            errno = EOVERFLOW;
            die("error: failed to resize array");
          }
        }
        tmptargets = realloc(targets, size * sizeof(target_t));
        if (tmptargets == NULL)
        {
          die("error: failed to allocate memory for input");
        }
        targets = tmptargets;
      }
      targets[n].target = strdup(line + i);
      if (targets[n].target == NULL)
      {
        die("error: failed to copy path");
      }
      targets[n].pattern = NULL;
      targets[n].uid = uid;
      targets[n].chown_uid = chown_uid;
      targets[n].gid = gid;
      targets[n].chown_gid = chown_gid;
      targets[n].mask = mask;
      targets[n].mask_d = mask_d;
      targets[n].mask_c = mask_c;
      targets[n].mask_b = mask_b;
      targets[n].mask_r = mask_r;
      targets[n].mask_f = mask_f;
      targets[n].mask_l = mask_l;
      targets[n].mask_s = mask_s;
      targets[n].chmod = chmod;
      targets[n].chmod_d = chmod_d;
      targets[n].chmod_c = chmod_c;
      targets[n].chmod_b = chmod_b;
      targets[n].chmod_r = chmod_r;
      targets[n].chmod_f = chmod_f;
      targets[n].chmod_l = chmod_l;
      targets[n].chmod_s = chmod_s;
      next_pattern = &(targets[n].pattern);
      initialized = 1;
    }
    else if (! initialized)
    {
      continue;
    }
    else if (line[0] == '+' || line[0] == '-')
    {
      if (line[1] != ' ')
      {
        die(
          "error: malformed input file: missing space after '%c'",
          line[0]
        );
      }
      if (line[0] == '+')
      {
        action = INCLUDE;
      }
      else
      {
        action = EXCLUDE;
      }
      * next_pattern = malloc(sizeof(pattern_t));
      if (* next_pattern == NULL)
      {
        die("error: failed to allocate memory for pattern struct");
      }
      for (j=2; line[j] != '\0'; j++)
      {
        if (line[j] == '\n')
        {
          line[j] = '\0';
          break;
        }
      }
      (* next_pattern)->pattern = strdup(line + 2);
      (* next_pattern)->action = action;
      (* next_pattern)->next = NULL;
      next_pattern = &((* next_pattern)->next);
    }
    else
    {
      continue;
    }
  }
  if (! is_stdin)
  {
    fclose(fd);
  }
  n ++;

  targets = realloc(targets, (n+1) * sizeof(target_t));
  if (targets == NULL)
  {
    die("error: failed to resize queue");
  }
  targets[n].target = NULL;

  return targets;
}



void
free_targets(target_t * targets)
{
  int i;
  pattern_t * pattern, * tmp;
  for (i=0; targets[i].target != NULL; i++)
  {
    free(targets[i].target);
    pattern = targets[i].pattern;
    while (pattern != NULL)
    {
      free(pattern->pattern);
      tmp = pattern->next;
      free(pattern);
      pattern = tmp;
    }
  }
  free(targets);
}



action_t
match_pattern_queue(pattern_t * pattern, char * path)
{
  action_t action;
  action = INCLUDE;

  while (pattern != NULL)
  {
    if (!fnmatch((* pattern).pattern, path, 0))
    {
      action = pattern->action;
    }
    pattern = pattern->next;
  }
  return action;
}


