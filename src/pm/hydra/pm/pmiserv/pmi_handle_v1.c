/* -*- Mode: C; c-basic-offset:4 ; -*- */
/*
 *  (C) 2008 by Argonne National Laboratory.
 *      See COPYRIGHT in top-level directory.
 */

#include "hydra.h"
#include "hydra_utils.h"
#include "bsci.h"
#include "pmi_handle.h"

static HYD_status fn_initack(int fd, char *args[])
{
    int id, rank, i;
    char *tmp[HYD_NUM_TMP_STRINGS], *cmd, *val;
    HYD_pmcd_pmi_pg_t *run;
    struct HYD_pmcd_token *tokens;
    int token_count;
    HYD_status status = HYD_SUCCESS;

    HYDU_FUNC_ENTER();

    status = HYD_pmcd_args_to_tokens(args, &tokens, &token_count);
    HYDU_ERR_POP(status, "unable to convert args to tokens\n");

    val = HYD_pmcd_find_token_keyval(tokens, token_count, "pmiid");
    HYDU_ERR_CHKANDJUMP(status, val == NULL, HYD_INTERNAL_ERROR,
                        "unable to find pmiid token\n");
    id = atoi(val);

    i = 0;
    tmp[i++] = HYDU_strdup("cmd=initack\ncmd=set size=");
    tmp[i++] = HYDU_int_to_str(HYD_pg_list->num_procs);
    tmp[i++] = HYDU_strdup("\ncmd=set rank=");

    status = HYD_pmcd_pmi_id_to_rank(id, &rank);
    HYDU_ERR_POP(status, "unable to convert ID to rank\n");
    tmp[i++] = HYDU_int_to_str(rank);

    tmp[i++] = HYDU_strdup("\ncmd=set debug=");
    tmp[i++] = HYDU_int_to_str(HYD_handle.user_global.debug);
    tmp[i++] = HYDU_strdup("\n");
    tmp[i++] = NULL;

    status = HYDU_str_alloc_and_join(tmp, &cmd);
    HYDU_ERR_POP(status, "error while joining strings\n");
    HYDU_free_strlist(tmp);

    status = HYDU_sock_writeline(fd, cmd, strlen(cmd));
    HYDU_ERR_POP(status, "error writing PMI line\n");

    HYDU_FREE(cmd);

    run = HYD_pg_list;
    while (run->next)
        run = run->next;

    /* Add the process to the last PG */
    status = HYD_pmcd_pmi_add_process_to_pg(run, fd, id);
    HYDU_ERR_POP(status, "unable to add process to pg\n");

  fn_exit:
    HYDU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

static HYD_status fn_get_maxes(int fd, char *args[])
{
    int i;
    char *tmp[HYD_NUM_TMP_STRINGS], *cmd;
    HYD_status status = HYD_SUCCESS;

    HYDU_FUNC_ENTER();

    i = 0;
    tmp[i++] = HYDU_strdup("cmd=maxes kvsname_max=");
    tmp[i++] = HYDU_int_to_str(MAXKVSNAME);
    tmp[i++] = HYDU_strdup(" keylen_max=");
    tmp[i++] = HYDU_int_to_str(MAXKEYLEN);
    tmp[i++] = HYDU_strdup(" vallen_max=");
    tmp[i++] = HYDU_int_to_str(MAXVALLEN);
    tmp[i++] = HYDU_strdup("\n");
    tmp[i++] = NULL;

    status = HYDU_str_alloc_and_join(tmp, &cmd);
    HYDU_ERR_POP(status, "unable to join strings\n");
    HYDU_free_strlist(tmp);

    status = HYDU_sock_writeline(fd, cmd, strlen(cmd));
    HYDU_ERR_POP(status, "error writing PMI line\n");
    HYDU_FREE(cmd);

  fn_exit:
    HYDU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

static HYD_status fn_get_appnum(int fd, char *args[])
{
    char *tmp[HYD_NUM_TMP_STRINGS], *cmd;
    int i;
    HYD_pmcd_pmi_process_t *process;
    HYD_status status = HYD_SUCCESS;

    HYDU_FUNC_ENTER();

    /* Find the group id corresponding to this fd */
    process = HYD_pmcd_pmi_find_process(fd);
    if (process == NULL)        /* We didn't find the process */
        HYDU_ERR_SETANDJUMP(status, HYD_INTERNAL_ERROR, "unable to find process structure\n");

    i = 0;
    tmp[i++] = HYDU_strdup("cmd=appnum appnum=");
    tmp[i++] = HYDU_int_to_str(process->node->pg->pgid);
    tmp[i++] = HYDU_strdup("\n");
    tmp[i++] = NULL;

    status = HYDU_str_alloc_and_join(tmp, &cmd);
    HYDU_ERR_POP(status, "unable to join strings\n");
    HYDU_free_strlist(tmp);

    status = HYDU_sock_writeline(fd, cmd, strlen(cmd));
    HYDU_ERR_POP(status, "error writing PMI line\n");
    HYDU_FREE(cmd);

  fn_exit:
    HYDU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

static HYD_status fn_get_my_kvsname(int fd, char *args[])
{
    char *tmp[HYD_NUM_TMP_STRINGS], *cmd;
    int i;
    HYD_pmcd_pmi_process_t *process;
    HYD_status status = HYD_SUCCESS;

    HYDU_FUNC_ENTER();

    /* Find the group id corresponding to this fd */
    process = HYD_pmcd_pmi_find_process(fd);
    if (process == NULL)        /* We didn't find the process */
        HYDU_ERR_SETANDJUMP1(status, HYD_INTERNAL_ERROR,
                             "unable to find process structure for fd %d\n", fd);

    i = 0;
    tmp[i++] = HYDU_strdup("cmd=my_kvsname kvsname=");
    tmp[i++] = HYDU_strdup(process->node->pg->kvs->kvs_name);
    tmp[i++] = HYDU_strdup("\n");
    tmp[i++] = NULL;

    status = HYDU_str_alloc_and_join(tmp, &cmd);
    HYDU_ERR_POP(status, "unable to join strings\n");
    HYDU_free_strlist(tmp);

    status = HYDU_sock_writeline(fd, cmd, strlen(cmd));
    HYDU_ERR_POP(status, "error writing PMI line\n");
    HYDU_FREE(cmd);

  fn_exit:
    HYDU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

static HYD_status fn_barrier_in(int fd, char *args[])
{
    HYD_pmcd_pmi_process_t *process, *prun;
    HYD_pmcd_pmi_node_t *node;
    const char *cmd;
    HYD_status status = HYD_SUCCESS;

    HYDU_FUNC_ENTER();

    /* Find the group id corresponding to this fd */
    process = HYD_pmcd_pmi_find_process(fd);
    if (process == NULL)        /* We didn't find the process */
        HYDU_ERR_SETANDJUMP1(status, HYD_INTERNAL_ERROR,
                             "unable to find process structure for fd %d\n", fd);

    process->node->pg->barrier_count++;

    /* All the processes have arrived at the barrier; send a
     * barrier_out message to everyone. */
    if (process->node->pg->barrier_count == process->node->pg->num_procs) {
        cmd = "cmd=barrier_out\n";
        for (node = process->node->pg->node_list; node; node = node->next) {
            for (prun = node->process_list; prun; prun = prun->next) {
                status = HYDU_sock_writeline(prun->fd, cmd, strlen(cmd));
                HYDU_ERR_POP(status, "error writing PMI line\n");
            }
        }

        process->node->pg->barrier_count = 0;
    }

  fn_exit:
    HYDU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

static HYD_status fn_put(int fd, char *args[])
{
    int i, ret;
    HYD_pmcd_pmi_process_t *process;
    char *kvsname, *key, *val;
    char *tmp[HYD_NUM_TMP_STRINGS], *cmd;
    struct HYD_pmcd_token *tokens;
    int token_count;
    HYD_status status = HYD_SUCCESS;

    HYDU_FUNC_ENTER();

    status = HYD_pmcd_args_to_tokens(args, &tokens, &token_count);
    HYDU_ERR_POP(status, "unable to convert args to tokens\n");

    kvsname = HYD_pmcd_find_token_keyval(tokens, token_count, "kvsname");
    HYDU_ERR_CHKANDJUMP(status, kvsname == NULL, HYD_INTERNAL_ERROR,
                        "unable to find token: kvsname\n");

    key = HYD_pmcd_find_token_keyval(tokens, token_count, "key");
    HYDU_ERR_CHKANDJUMP(status, key == NULL, HYD_INTERNAL_ERROR,
                        "unable to find token: key\n");

    val = HYD_pmcd_find_token_keyval(tokens, token_count, "value");
    HYDU_ERR_CHKANDJUMP(status, val == NULL, HYD_INTERNAL_ERROR,
                        "unable to find token: val\n");

    /* Find the group id corresponding to this fd */
    process = HYD_pmcd_pmi_find_process(fd);
    if (process == NULL)        /* We didn't find the process */
        HYDU_ERR_SETANDJUMP1(status, HYD_INTERNAL_ERROR,
                             "unable to find process structure for fd %d\n", fd);

    if (strcmp(process->node->pg->kvs->kvs_name, kvsname))
        HYDU_ERR_SETANDJUMP2(status, HYD_INTERNAL_ERROR,
                             "kvsname (%s) does not match this process' kvs space (%s)\n",
                             kvsname, process->node->pg->kvs->kvs_name);

    status = HYD_pmcd_pmi_add_kvs(key, val, process->node->pg->kvs, &ret);
    HYDU_ERR_POP(status, "unable to add keypair to kvs\n");

    i = 0;
    tmp[i++] = HYDU_strdup("cmd=put_result rc=");
    tmp[i++] = HYDU_int_to_str(ret);
    if (ret == 0) {
        tmp[i++] = HYDU_strdup(" msg=success");
    }
    else {
        tmp[i++] = HYDU_strdup(" msg=duplicate_key");
        tmp[i++] = HYDU_strdup(key);
    }
    tmp[i++] = HYDU_strdup("\n");
    tmp[i++] = NULL;

    status = HYDU_str_alloc_and_join(tmp, &cmd);
    HYDU_ERR_POP(status, "unable to join strings\n");
    HYDU_free_strlist(tmp);

    status = HYDU_sock_writeline(fd, cmd, strlen(cmd));
    HYDU_ERR_POP(status, "error writing PMI line\n");
    HYDU_FREE(cmd);

  fn_exit:
    HYDU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

static HYD_status fn_get(int fd, char *args[])
{
    int i, found, ret;
    HYD_pmcd_pmi_process_t *process;
    HYD_pmcd_pmi_kvs_pair_t *run;
    char *kvsname, *key, *node_list;
    char *tmp[HYD_NUM_TMP_STRINGS], *cmd;
    struct HYD_pmcd_token *tokens;
    int token_count;
    HYD_status status = HYD_SUCCESS;

    HYDU_FUNC_ENTER();

    status = HYD_pmcd_args_to_tokens(args, &tokens, &token_count);
    HYDU_ERR_POP(status, "unable to convert args to tokens\n");

    kvsname = HYD_pmcd_find_token_keyval(tokens, token_count, "kvsname");
    HYDU_ERR_CHKANDJUMP(status, kvsname == NULL, HYD_INTERNAL_ERROR,
                        "unable to find token: kvsname\n");

    key = HYD_pmcd_find_token_keyval(tokens, token_count, "key");
    HYDU_ERR_CHKANDJUMP(status, key == NULL, HYD_INTERNAL_ERROR,
                        "unable to find token: key\n");

    /* Find the group id corresponding to this fd */
    process = HYD_pmcd_pmi_find_process(fd);
    if (process == NULL)        /* We didn't find the process */
        HYDU_ERR_SETANDJUMP1(status, HYD_INTERNAL_ERROR,
                             "unable to find process structure for fd %d\n", fd);

    if (strcmp(process->node->pg->kvs->kvs_name, kvsname))
        HYDU_ERR_SETANDJUMP2(status, HYD_INTERNAL_ERROR,
                             "kvsname (%s) does not match this process' kvs space (%s)\n",
                             kvsname, process->node->pg->kvs->kvs_name);

    /* Try to find the key */
    found = 0;
    for (run = process->node->pg->kvs->key_pair; run; run = run->next) {
        if (!strcmp(run->key, key)) {
            found = 1;
            break;
        }
    }

    if (found == 0) {
        /* Didn't find the job attribute; see if we know how to
         * generate it */
        if (strcmp(key, "PMI_process_mapping") == 0) {
            /* Create a vector format */
            status = HYD_pmcd_pmi_process_mapping(process, HYD_pmcd_pmi_vector, &node_list);
            HYDU_ERR_POP(status, "Unable to get process mapping information\n");

            /* Make sure the node list is within the size allowed by
             * PMI. Otherwise, tell the client that we don't know what
             * the key is */
            if (strlen(node_list) <= MAXVALLEN) {
                status = HYD_pmcd_pmi_add_kvs("PMI_process_mapping", node_list,
                                              process->node->pg->kvs, &ret);
                HYDU_ERR_POP(status, "unable to add process_mapping to KVS\n");
            }

            HYDU_FREE(node_list);
        }

        /* Search for the key again */
        for (run = process->node->pg->kvs->key_pair; run; run = run->next) {
            if (!strcmp(run->key, key)) {
                found = 1;
                break;
            }
        }
    }

    i = 0;
    tmp[i++] = HYDU_strdup("cmd=get_result rc=");
    if (found) {
        tmp[i++] = HYDU_strdup("0 msg=success value=");
        tmp[i++] = HYDU_strdup(run->val);
    }
    else {
        tmp[i++] = HYDU_strdup("-1 msg=key_");
        tmp[i++] = HYDU_strdup(key);
        tmp[i++] = HYDU_strdup("_not_found value=unknown");
    }
    tmp[i++] = HYDU_strdup("\n");
    tmp[i++] = NULL;

    status = HYDU_str_alloc_and_join(tmp, &cmd);
    HYDU_ERR_POP(status, "unable to join strings\n");
    HYDU_free_strlist(tmp);

    status = HYDU_sock_writeline(fd, cmd, strlen(cmd));
    HYDU_ERR_POP(status, "error writing PMI line\n");
    HYDU_FREE(cmd);

  fn_exit:
    HYDU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

static HYD_status fn_finalize(int fd, char *args[])
{
    const char *cmd;
    HYD_status status = HYD_SUCCESS;

    HYDU_FUNC_ENTER();

    cmd = "cmd=finalize_ack\n";
    status = HYDU_sock_writeline(fd, cmd, strlen(cmd));
    HYDU_ERR_POP(status, "error writing PMI line\n");

    if (status == HYD_SUCCESS) {
        status = HYDU_dmx_deregister_fd(fd);
        HYDU_ERR_POP(status, "unable to register fd\n");
        close(fd);
    }

  fn_exit:
    HYDU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

static HYD_status fn_get_usize(int fd, char *args[])
{
    int usize, i;
    char *tmp[HYD_NUM_TMP_STRINGS], *cmd;
    HYD_status status = HYD_SUCCESS;

    HYDU_FUNC_ENTER();

    status = HYDT_bsci_query_usize(&usize);
    HYDU_ERR_POP(status, "unable to get bootstrap universe size\n");

    i = 0;
    tmp[i++] = HYDU_strdup("cmd=universe_size size=");
    tmp[i++] = HYDU_int_to_str(usize);
    tmp[i++] = HYDU_strdup("\n");
    tmp[i++] = NULL;

    status = HYDU_str_alloc_and_join(tmp, &cmd);
    HYDU_ERR_POP(status, "unable to join strings\n");
    HYDU_free_strlist(tmp);

    status = HYDU_sock_writeline(fd, cmd, strlen(cmd));
    HYDU_ERR_POP(status, "error writing PMI line\n");
    HYDU_FREE(cmd);

  fn_exit:
    HYDU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

static HYD_status fn_spawn(int fd, char *args[])
{
    struct HYD_pg *pg;
    HYD_pmcd_pmi_pg_t *pmi_pg;
    struct HYD_node *node, *start_node;
    struct HYD_proxy *proxy;
    int offset, nprocs, procs_left;
    struct HYD_pmcd_token *tokens;
    int token_count, total_nodes, i, start_pid, num_procs;
    char *val, *execname;
    HYD_status status = HYD_SUCCESS;

    HYDU_FUNC_ENTER();

    HYDU_print_strlist(args);

    status = HYD_pmcd_args_to_tokens(args, &tokens, &token_count);
    HYDU_ERR_POP(status, "unable to convert args to tokens\n");

    val = HYD_pmcd_find_token_keyval(tokens, token_count, "nprocs");
    HYDU_ERR_CHKANDJUMP(status, val == NULL, HYD_INTERNAL_ERROR,
                        "unable to find token: nprocs\n");
    nprocs = atoi(val);

    val = HYD_pmcd_find_token_keyval(tokens, token_count, "argcnt");
    HYDU_ERR_CHKANDJUMP(status, val == NULL, HYD_INTERNAL_ERROR,
                        "unable to find token: argcnt\n");
    if (atoi(val))
        HYDU_ERR_SETANDJUMP(status, HYD_INTERNAL_ERROR,
                            "arguments not supported in spawn\n");

    execname = HYD_pmcd_find_token_keyval(tokens, token_count, "execname");
    HYDU_ERR_CHKANDJUMP(status, execname == NULL, HYD_INTERNAL_ERROR,
                        "unable to find token: execname\n");

    /* Find the index of the pgid that we need to create */
    for (pg = &HYD_handle.pg_list; pg->next; pg = pg->next);
    status = HYDU_alloc_pg(&pg->next);
    HYDU_ERR_POP(status, "unable to allocate process group\n");

    pg->next->pgid = pg->pgid + 1;
    pg->next->pg_process_count = nprocs;

    /* Find which node we start on */
    offset = 0;
    for (pg = &HYD_handle.pg_list; pg->next; pg = pg->next)
        offset += pg->pg_process_count;

    offset %= HYD_handle.global_core_count;
    for (node = HYD_handle.node_list; node; node = node->next) {
        offset -= node->core_count;
        if (offset < 0)
            break;
    }
    start_node = node;

    /* Check how many total number of nodes we have */
    for (total_nodes = 0, node = HYD_handle.node_list; node; node = node->next)
        total_nodes++;

    /* Run to the last PG */
    for (pg = &HYD_handle.pg_list; pg->next; pg = pg->next);

    /* Create proxies for this group */
    start_pid = 0;
    procs_left = nprocs;
    for (i = 0, node = start_node; i < total_nodes; i++) {
        /* Make sure we need more proxies */
        if (nprocs < HYD_handle.global_core_count && nprocs >= start_pid)
            break;

        if (pg->proxy_list == NULL) {
            status = HYDU_alloc_proxy(&pg->proxy_list);
            HYDU_ERR_POP(status, "unable to allocate proxy\n");
            proxy = pg->proxy_list;
        }
        else {
            status = HYDU_alloc_proxy(&proxy->next);
            HYDU_ERR_POP(status, "unable to allocate proxy\n");
            proxy = proxy->next;
        }

        /* For the first node, use only the remaining cores. For the
         * last node, we need to make sure its not oversubscribed
         * since the first proxy we started on might repeat. */
        if (i == 0)
            proxy->node.core_count = -(offset); /* offset is negative */
        else if (i == (total_nodes - 1))
            proxy->node.core_count = node->core_count + offset;
        else
            proxy->node.core_count = node->core_count;

        proxy->proxy_id = i;
        proxy->start_pid = start_pid;
        proxy->node.hostname = HYDU_strdup(node->hostname);
        proxy->node.next = NULL;

        /* Add the executable information in the proxy */
        status = HYDU_alloc_proxy_exec(&proxy->exec_list);
        HYDU_ERR_POP(status, "unable to allocate proxy exec\n");

        proxy->exec_list->exec[0] = HYDU_strdup(execname);
        proxy->exec_list->exec[1] = NULL;

        /* Figure out how many processes to set on this proxy */
        num_procs = (pg->pg_process_count / HYD_handle.global_core_count) *
            HYD_handle.global_core_count;
        if (nprocs % HYD_handle.global_core_count >= start_pid) {
            if (procs_left > proxy->node.core_count)
                num_procs += proxy->node.core_count;
            else
                num_procs += procs_left;
        }

        proxy->exec_list->proc_count = num_procs;
        proxy->proxy_process_count += num_procs;

        /* It is not clear what kind of environment needs to get
         * passed to the spawned process. Don't set anything here, and
         * let the proxy do whatever it does by default. */
        proxy->exec_list->env_prop = NULL;
        proxy->exec_list->user_env = NULL;

        /* If we found enough proxies, break out */
        start_pid += proxy->node.core_count;
        procs_left -= proxy->node.core_count;
        if (procs_left <= 0)
            break;

        node = node->next;
        /* Handle the wrap around case for the nodes */
        if (node == NULL)
            node = HYD_handle.node_list;
    }

    /* Create a new PMI process group */
    for (pmi_pg = HYD_pg_list; pmi_pg->next; pmi_pg = pmi_pg->next);
    status = HYD_pmcd_create_pg(&pmi_pg->next, pmi_pg->pgid + 1);
    HYDU_ERR_POP(status, "unable to create PMI pg\n");

    /*
     * TODO:
     *   1. Create a PMI port to listen on
     *   2. Spawn the requested processes
     *   3. Modify the callback code to point to the appropriate process group
     */

    HYDU_ERR_SETANDJUMP(status, HYD_INTERNAL_ERROR, "exit\n");

  fn_exit:
    HYDU_FUNC_EXIT();
    return status;

  fn_fail:
    goto fn_exit;
}

/* TODO: abort, create_kvs, destroy_kvs, getbyidx */
static struct HYD_pmcd_pmi_handle_fns pmi_v1_handle_fns_foo[] = {
    {"initack", fn_initack},
    {"get_maxes", fn_get_maxes},
    {"get_appnum", fn_get_appnum},
    {"get_my_kvsname", fn_get_my_kvsname},
    {"barrier_in", fn_barrier_in},
    {"put", fn_put},
    {"get", fn_get},
    {"get_universe_size", fn_get_usize},
    {"finalize", fn_finalize},
    {"spawn", fn_spawn},
    {"\0", NULL}
};

struct HYD_pmcd_pmi_handle HYD_pmcd_pmi_v1 = { PMI_V1_DELIM, pmi_v1_handle_fns_foo };
