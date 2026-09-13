#include "kernel/types.h"
#include "user/user.h"

#define NPROC 4
#define NRES  3

int allocation[NPROC][NRES];
int request[NPROC][NRES];

int waitfor[NPROC][NPROC];

int visited[NPROC];
int in_stack[NPROC];
int path[NPROC + 1];
int path_len;

void
print_matrix(char *name, int mat[NPROC][NRES], int cols)
{
  printf("%s\n     ", name);
  for (int j = 0; j < cols; j++)
    printf(" R%d", j);
  printf("\n");
  for (int i = 0; i < NPROC; i++) {
    printf("  P%d ", i);
    for (int j = 0; j < cols; j++)
      printf("  %d", mat[i][j]);
    printf("\n");
  }
}

void
build_waitfor_graph(void)
{
  for (int i = 0; i < NPROC; i++)
    for (int k = 0; k < NPROC; k++)
      waitfor[i][k] = 0;

  for (int i = 0; i < NPROC; i++) {
    for (int j = 0; j < NRES; j++) {
      if (request[i][j]) {
        for (int k = 0; k < NPROC; k++) {
          if (k != i && allocation[k][j]) {
            waitfor[i][k] = 1;
          }
        }
      }
    }
  }
}

void
print_waitfor_graph(void)
{
  printf("Wait-for graph edges:\n");
  int any = 0;
  for (int i = 0; i < NPROC; i++) {
    for (int k = 0; k < NPROC; k++) {
      if (waitfor[i][k]) {
        printf("  P%d -> P%d\n", i, k);
        any = 1;
      }
    }
  }
  if (!any)
    printf("  (no edges)\n");
}

int
dfs(int u)
{
  visited[u] = 1;
  in_stack[u] = 1;
  path[path_len++] = u;

  for (int v = 0; v < NPROC; v++) {
    if (!waitfor[u][v])
      continue;
    if (in_stack[v]) {

      path[path_len++] = v;
      return 1;
    }
    if (!visited[v]) {
      if (dfs(v))
        return 1;
    }
  }

  in_stack[u] = 0;
  path_len--;
  return 0;
}

int
detect_deadlock(void)
{
  for (int i = 0; i < NPROC; i++) {
    visited[i] = 0;
    in_stack[i] = 0;
  }

  for (int start = 0; start < NPROC; start++) {
    if (visited[start])
      continue;
    path_len = 0;
    if (dfs(start)) {

      int last = path[path_len - 1];
      int start_idx = 0;
      for (int i = 0; i < path_len - 1; i++) {
        if (path[i] == last) {
          start_idx = i;
          break;
        }
      }
      printf("Deadlock DETECTED. Cycle: ");
      for (int i = start_idx; i < path_len; i++) {
        printf("P%d", path[i]);
        if (i != path_len - 1)
          printf(" -> ");
      }
      printf("\n");
      return 1;
    }
  }

  printf("No deadlock detected (wait-for graph is acyclic).\n");
  return 0;
}

void
clear_scenario(void)
{
  for (int i = 0; i < NPROC; i++)
    for (int j = 0; j < NRES; j++) {
      allocation[i][j] = 0;
      request[i][j] = 0;
    }
}

void
run_scenario_a(void)
{

  clear_scenario();
  allocation[0][0] = 1;
  request[0][1] = 1;

  allocation[1][1] = 1;
  request[1][2] = 1;

  allocation[2][2] = 1;

  request[3][0] = 1;

  printf("\n===== Scenario A (expected: no deadlock) =====\n");
  print_matrix("Allocation", allocation, NRES);
  print_matrix("Request", request, NRES);
  build_waitfor_graph();
  print_waitfor_graph();
  detect_deadlock();
}

void
run_scenario_b(void)
{

  clear_scenario();
  allocation[0][0] = 1;
  request[0][1] = 1;

  allocation[1][1] = 1;
  request[1][2] = 1;

  allocation[2][2] = 1;
  request[2][0] = 1;

  request[3][1] = 1;

  printf("\n===== Scenario B (expected: deadlock) =====\n");
  print_matrix("Allocation", allocation, NRES);
  print_matrix("Request", request, NRES);
  build_waitfor_graph();
  print_waitfor_graph();
  detect_deadlock();
}

int
main(int argc, char *argv[])
{
  printf("===== Deadlock Detection (Wait-For Graph) =====\n");
  run_scenario_a();
  run_scenario_b();
  exit(0);
}
