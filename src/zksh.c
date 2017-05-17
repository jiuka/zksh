/* 
 *  zksh - ZooKeeper Shell utils.
 *
 *  Copyright (C) 2017 Marius Rieder
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "zksh.h"

#include <stdlib.h>
#include <getopt.h>
#include <libconfig.h>
#include <zookeeper/zookeeper.h>
#include <string.h>

static struct option const zk_long_options[] = {
  {"config", required_argument, 0, 'c'},
  {"server", required_argument, 0, 's'},
  {"auth", required_argument, 0, 'a'},
  {"acl", required_argument, 0, 'A'},
  {"quiet", no_argument, 0, 'q'},
  {"verbose", no_argument, 0, 'v'},
  {"version", no_argument, 0, 'V'},
  {"help", no_argument, 0, 'h'},
  {0, 0, 0, 0}
};

const char *zksh_config;
const char *zksh_program;
const char *zksh_server;
const char *zksh_auth;
struct String_vector zksh_node_list;
const char *zksh_acl;
int zksh_loglevel = ZOO_LOG_LEVEL_WARN;
zhandle_t *zh;
static clientid_t zksh_myid;


int read_config() {
  config_t cfg;
  config_setting_t *setting;
  const char *str;

  config_init(&cfg);

  /* Read the file. If there is an error, report it and exit. */
  if(!config_read_file(&cfg, zksh_config)) {

    fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
        config_error_line(&cfg), config_error_text(&cfg));
    config_destroy(&cfg);
    return(EXIT_FAILURE);
  }

  /* Get the store name. */
  if(zksh_server == NULL && config_lookup_string(&cfg, "server", &str))
    zksh_server = strdup(str);

  if(zksh_auth == NULL && config_lookup_string(&cfg, "auth", &str))
    zksh_auth = strdup(str);

  config_destroy(&cfg);

  return 0;
}

void zk_version() {
  printf("%s - ZooKeeper Shell utils. v" PACKAGE_VERSION "\n", zksh_program);
  printf("   libconfig v%d.%d.%d\n", LIBCONFIG_VER_MAJOR, LIBCONFIG_VER_MINOR, LIBCONFIG_VER_REVISION);
}

int zksh_init(int *argc, char **argv) {

  zksh_program = argv[0];

  int argc_new = 0;
  char **argv_new = NULL;

  int c, strip_args;
  zksh_config = NULL;
  zksh_node_list.count = 0;
  zksh_node_list.data = NULL;

  while ((c = getopt_long (*argc, argv, ":c:s:a:A:qvVh", zk_long_options,(int *) 0)) != EOF) {
    strip_args = 0;
    switch(c) {
      case 'c':
        zksh_config = optarg;
        strip_args = 2;
        break;
      case 's':
        zksh_server = optarg;
        strip_args = 2;
        break;
      case 'a':
        zksh_auth = optarg;
        strip_args = 2;
        break;
      case 'A':
        zksh_acl = optarg;
        strip_args = 2;
        break;
      case 'q':
        if (zksh_loglevel > ZOO_LOG_LEVEL_ERROR)
          zksh_loglevel--;
        strip_args = 1;
        break;
      case 'v':
        if (zksh_loglevel < ZOO_LOG_LEVEL_DEBUG)
          zksh_loglevel++;
        strip_args = 1;
        break;
      case 'V':
        zk_version();
        exit(0);
        break;
      case 'h':
        zk_usage(EXIT_SUCCESS);
        break;
    }

    if (strip_args > 0) {
      optind-=strip_args;
      *argc-=strip_args;
      for(int i=optind; i<=*argc; i++) {
        argv[i] = argv[i+strip_args];
      }
    }
  }

  // Reset getopt
  optind=1;

  // Read Config
  if (zksh_config == NULL) {
    zksh_config = secure_getenv("ZKSH_CONFIG");
  }
  if (zksh_config != NULL) {
    read_config();
  }

  // Read Env
  if (zksh_server == NULL) {
    zksh_server = secure_getenv("ZKSH_SERVER");
  }

  if (zksh_server == NULL) {
    zk_usage(EXIT_FAILURE);
  }

  return 0;
}

void zksh_watcher(zhandle_t *zzh, int type, int state, const char *path,
    void* context) {
  if (zksh_loglevel >= ZOO_LOG_LEVEL_INFO)
    printf("WATCHER[%s]: type=%d, state=%d\n", path, type, state);
}

void zksh_disconnect() {
  if (zh == NULL)
    return;

  zookeeper_close(zh);
  zh = NULL;
}

int zksh_connect() {
  zoo_set_debug_level(zksh_loglevel);
  zh = zookeeper_init(zksh_server, zksh_watcher, 30000, &zksh_myid, NULL, 0);

  if (zksh_auth != NULL)
    zoo_add_auth(zh, "digest", zksh_auth, strlen(zksh_auth), NULL, NULL);

  atexit(zksh_disconnect);
}

void zk_usage(int status) {
  printf("%s - ZooKeeper Shell utils.\n", zksh_program);
  printf("Usage: %s [OPTION] NODE\n\n", zksh_program);

  printf("%s\n", usage_description);

  printf("\
  ZKSH Options:\n\
    -s, --server=STRING   ZooKeeper server string.\n\
    -a, --auth=STRING     ZooKeeper auth string.\n\
    -A, --acl=STRING      ZooKeeper acl string to be added to created nodes.\n\
    -q, --quiet           Print less output.\n\
    -v, --verbose         Print more output.\n\
    -V, --version         Show the program version.\n\
    -h, --help            Show this Help.\n\
  ");

  exit(status);
}

int zk_return_code = EXIT_SUCCESS;
void zk_check_rc(int rc, char *node) {
  if (rc == ZCONNECTIONLOSS) {
    fprintf(stderr, "%s: %s: Connection loss\n", zksh_program, node);
    exit(EXIT_FAILURE);
  } else if (rc == ZNONODE) {
    fprintf(stderr, "%s: %s: No such node\n", zksh_program, node);
    zk_return_code = EXIT_FAILURE;
  } else if (rc == ZNOAUTH) {
    fprintf(stderr, "%s: %s: Permission denied\n", zksh_program, node);
    zk_return_code = EXIT_FAILURE;
  } else if (rc == ZBADVERSION) {
    fprintf(stderr, "%s: %s: Node version missmatch\n", zksh_program, node);
    zk_return_code = EXIT_FAILURE;
  } else if (rc == ZNODEEXISTS) {
    fprintf(stderr, "%s: %s: Node already exists\n", zksh_program, node);
    zk_return_code = EXIT_FAILURE;
  } else if (rc == ZNOTEMPTY) {
    fprintf(stderr, "%s: %s: Node is not empty\n", zksh_program, node);
    zk_return_code = EXIT_FAILURE;
  } else if (rc == ZBADARGUMENTS) {
    fprintf(stderr, "%s: %s: Invalid input parameters\n", zksh_program, node);
    zk_return_code = EXIT_FAILURE;
  } else if (rc == ZINVALIDSTATE) {
    fprintf(stderr, "%s: %s: Invalid zookeeper state\n", zksh_program, node);
    zk_return_code = EXIT_FAILURE;
  } else if (rc == ZMARSHALLINGERROR) {
    fprintf(stderr, "%s: %s: Marshall request failed\n", zksh_program, node);
    zk_return_code = EXIT_FAILURE;
  } else if (rc != ZOK) {
    fprintf(stderr, "%s: %s: Unknown error %d\n", zksh_program, node, rc);
    zk_return_code = EXIT_FAILURE;
  }
}

int zksh_check_nodepath(char *node) {
  if(node[0] != '/') {
    fprintf(stderr, "%s: Node must start with /, found: %s\n", zksh_program, node);
    return 1;
  }
  return 0;
}

char *zksh_join_path(const char *path, const char *node) {
  int rc;
  char *new;

  if (path[0]+strlen(path)-1 == '/')
    rc = asprintf(&new, "%s%s", path, node);
  else
    rc = asprintf(&new, "%s/%s", path, node);
  if (rc == -1) {
    fprintf(stderr, "%s: asprintf failed.\n", zksh_program);
    exit(EXIT_FAILURE);
  }
  return new;
}

struct ACL_vector *aclv = NULL;
struct ACL_vector *zksh_get_acl() {
  if (zksh_acl != NULL) {
    if (aclv == NULL) {
      aclv = malloc(sizeof(struct ACL_vector));
      aclv->count = 2;
      aclv->data = malloc(sizeof(struct ACL)*2);

      aclv->data[0].perms = ZOO_PERM_ALL;
      aclv->data[0].id.scheme = strdup("digest");
      aclv->data[0].id.id = strdup(zksh_acl);

      aclv->data[1].perms = ZOO_PERM_READ;
      aclv->data[1].id.scheme = strdup("world");
      aclv->data[1].id.id = strdup("anyone");
    }
    return aclv;
  } else {
    return &ZOO_OPEN_ACL_UNSAFE;
  }
}
