# test_3d_game

## Profile
```bash
cd $BUILD_DIR
sudo sysctl kernel.perf_event_paranoid=2
perf record -g ./test_3d_game
flamegraph --perfdata ./perf.data
```
