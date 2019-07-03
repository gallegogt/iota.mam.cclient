# [UNOFICIAL] Client for Entangled/MAM API


Examples:

SEND MSGS:

```
bazel run -c opt -- //mam/examples:send-message nodes.thetangle.org 443  ZGJAUEQWDWGZOUPXGQSEPVHREFQA9XCMEIZVZNOIT9AXZLJSBEKEFZWHWDNPGRCBBVMBUEMENYEPDDVZC "{payload: {"is_json": true}}" yes
```

RECEIVE:

```
 bazel run -c opt -- //mam/examples:receive nodes.thetangle.org 443 <BUNDLE>
```


