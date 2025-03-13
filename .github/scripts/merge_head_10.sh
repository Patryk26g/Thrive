#!/bin/bash
# Custom merge driver: Keeps the first 10 lines from the incoming branch, merges the rest normally.

current="$1"   # Ours (local branch)
incoming="$2"  # Theirs (incoming branch)
ancestor="$3"  # Base (common ancestor)

# Take first 10 lines from the incoming branch
head -n 10 "$incoming" > merged_file

# Merge lines 11+ using Git's built-in merge tool
tail -n +11 "$ancestor" > ancestor_tail
tail -n +11 "$current" > current_tail
tail -n +11 "$incoming" > incoming_tail

git merge-file -p current_tail ancestor_tail incoming_tail >> merged_file

# Replace the original file with the merged content
mv merged_file "$current"
