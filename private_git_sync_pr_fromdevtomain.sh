#!/bin/bash

# 遇到错误立即停止
set -e

# 获取远程最新状态
git fetch origin

# 切换到 main 分支并同步远程的 main
git checkout main && git pull origin main

# 切换到 dev 分支并同步远程的 dev
git checkout dev && git reset --hard main && git push origin dev -f

echo "dev 已强制重置为 main，所有 dev 独有提交已丢失！"