#!/bin/bash
set -e

WREN_PY="python3"
if [ -n "$WREN_PY_BINARY" ]
then
  WREN_PY="$WREN_PY_BINARY"
fi

# Build the docs.
mkdir -p build
$WREN_PY ./util/generate_docs.py
cp -r build/docs/. build/gh-pages

# Clone the repo at the gh-pages branch.
git clone https://${GH_TOKEN}@github.com/${TRAVIS_REPO_SLUG} gh-pages-repo \
    --branch gh-pages --depth 1
cd gh-pages-repo

# Copy them into the gh-pages branch.
rm -rf *
cp -r ../build/gh-pages/* .

# Restore CNAME file that gets deleted by `rm -rf *`.
echo "wren.io" > "CNAME"

git status
ls

if ! $( git diff-index --quiet HEAD ) ; then
  git config user.name "Travis CI"
  git config user.email "$COMMIT_AUTHOR_EMAIL"
  git add --all .
  git commit -m "Deploy to GitHub Pages: ${SHA}"
  git push
fi
