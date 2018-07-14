#!/bin/bash
set -e

# Install the Wren Pygments lexer.
cd util/pygments-lexer
sudo python setup.py develop
cd ../..

# Build the docs.
make gh-pages

# Clone the repo at the gh-pages branch.
# TODO: Testing right now, so not using live branch.
git clone https://${GH_TOKEN}@github.com/${TRAVIS_REPO_SLUG} gh-pages-repo \
    --branch gh-pages-temp --depth 1
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
