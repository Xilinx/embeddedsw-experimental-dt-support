import os
import sys
import glob
import fileinput
import shutil
import yaml
import subprocess
from pathlib import Path
from distutils.dir_util import copy_tree
from typing import Any, List, Optional, Dict
from collections.abc import MutableMapping

def delete_keys_from_dict(dictionary, keys):
    keys_set = set(keys)  # Just an optimization for the "if key in keys" lookup.

    modified_dict = {}
    for key, value in dictionary.items():
        if key not in keys_set:
            if isinstance(value, MutableMapping):
                modified_dict[key] = delete_keys_from_dict(value, keys_set)
            else:
                modified_dict[key] = value  # or copy.deepcopy(value) if a copy is desired for non-dicts.
    return modified_dict


def is_file(filepath: str, silent_discard: bool = True) -> bool:
    """Return True if the file exists Else returns False and raises Not Found Error Message.
    Args:
        filepath: File Path.
    Raises:
        FileNotFoundError: Raises exception if file not found.
    Returns:
        bool: True, if file is found Or False, if file is not found.
    """
   
    if os.path.isfile(filepath):
        return True
    elif not silent_discard:
        err_msg = f"No such file exists: {filepath}"
        raise FileNotFoundError(err_msg) from None
    else:
        return False

def is_dir(dirpath: str, silent_discard: bool = True) -> bool:
    """Checks if directory exists.
    Args:
        dirpath: Directory Path.
    Raises:
        ValueError (Exception): Raises exception if directory not found.
    Returns:
        bool: True, if directory is found Or False, if directory is not found.
    """

    if os.path.isdir(dirpath):
        return True
    elif not silent_discard:
        err_msg = f"No such directory exists: {dirpath}"
        raise ValueError(err_msg) from None
    else:
        return False


def remove(path: str, silent_discard: bool = True) -> None:
    """Removes any file or folder recursively, if it exists else reports error message based on user demand.
    Args:
        path: Directory or file path.
    """
    is_successful = False
    try:
        if is_dir(path):
            os.sync()
            shutil.rmtree(path)
            is_successful = True
        else:
            os.remove(path)
            is_successful = True
    except OSError as e:
        pass
    if not silent_discard:
        if is_successful:
            print("%s Removed successfully" % path)
        else:
            print("Error: %s" % path, e.strerror)  # FIXME: e doesn't exist here
            sys.exit(1)

    #FIXME: Fix this API
    

def mkdir(folderpath: str, silent_discard: bool = True) -> None:
    """Create the folder structure, raises Error Message on demand.
    Args:
        folderpath: Path of the folder structure.
    """
    is_successful = False
    try:
        os.makedirs(folderpath)
        is_successful = True
    except:
        pass

    if not silent_discard:
        if is_successful:
            print("%s Directory created " % folderpath)
        else:
            print("%s Unable to create directory " % folderpath)
            sys.exit(1)

def copy_file(src: str, dest, follow_symlinks=False, silent_discard=True):
    is_file(src, silent_discard)
    shutil.copy2(src, dest, follow_symlinks=follow_symlinks)

def copy_directory(src: str, dst: str, symlinks: bool = False, ignore=None) -> None:
    if not is_dir(dst):
        mkdir(dst)
    for item in os.listdir(src):
        s = os.path.join(src, item)
        d = os.path.join(dst, item)
        if is_dir(s):
            if is_dir(d):
                copy_tree(s, d, symlinks)
            else:
                shutil.copytree(s, d, symlinks, ignore)
        else:
            shutil.copy2(s, d)


def copy_data(
    src: str, dst: str, symlinks: bool = False, ignore=None, silent_discard: bool = True
) -> None:
    """This api finds if the source is a file or a directory, and copies it accordingly
    to the destination directory"""
    if is_file(src):
        copy_file(src, dst, follow_symlinks=symlinks)
    elif is_dir(src):
        copy_directory(src, dst, symlinks, ignore)
    else:
        err_msg = f"The path: {src} doesn't exist"
        if not silent_discard:
            raise ValueError(err_msg)

def reset(path: str) -> None:
    remove(path)
    mkdir(path)

def validate_if_not_exist(config_file, dir_type, dir_name):
    assert is_file(config_file), f"{dir_type.title()} with the name {dir_name} doesnt exist. Create the {dir_type} first."

def validate_if_exist(config_file, dir_type, dir_name):
    assert not is_file(config_file), f"{dir_type.title()} with name {dir_name} already exists. Cannot create a new {dir_type} with same name."

def fetch_yaml_data(config_file, dir_type):
    assert is_file(config_file), f"Could not find valid {dir_type} at {get_dir_path(config_file)}"
    data = load_yaml(config_file)
    return data

def load_yaml(filepath: str) -> Optional[dict]:
    """Read yaml file data and returns data in a dict format.
    Args:
        filepath: Path of the yaml file.
    Returns:
        dict: Return Python dict if the file reading is successful.
    """

    if is_file(filepath):
        try:
            with open(filepath) as f:
                data = yaml.safe_load(f)
            return data
        except:
            print("%s file reading failed" % filepath)
            return None
    else:
        return None

def write_yaml(filepath: str, data):
    with open(filepath, 'w') as outfile:
        yaml.dump(data, outfile, default_flow_style=False, sort_keys=False)

def update_yaml(filepath: str, dir_type, key, data, action="add"):
    new_data = fetch_yaml_data(filepath, dir_type)
    if action == "add":
        try:
            if isinstance(data, dict):
                new_data[key] = {**new_data[key] , **data}
            elif isinstance(data, list):
                new_data[key] += data
            else:
                new_data[key] = data
        except KeyError:
            new_data[key] = data
    elif action == "remove":
        new_data = delete_keys_from_dict(new_data, key)

    write_yaml(filepath, new_data)

def add_newline(File, newline):
    with open(File, "a") as fd:
        fd.write(newline + "\n")

def remove_line(File, match_string) -> None:
    with open(File, "r+") as f:
        new_f = f.readlines()
        f.seek(0)
        for line in new_f:
            if match_string not in line:
                f.write(line)
        f.truncate()

def replace_line(File: str, search_string: str, add_line: str) -> None:
    if is_file(File) == True:
        add_line = add_line + "\n"
        for line in fileinput.input(File, inplace=True):
            if search_string in line:
                line = add_line
            sys.stdout.write(line)
    else:
        err_msg = f"No such file exists: {File}"
        raise FileNotFoundError(err_msg)

def runcmd(cmd, logfile=None) -> bool:

    ret = True
    if logfile is None:
        try:
            subprocess.check_call(cmd, shell=True)
        except subprocess.CalledProcessError:
            ret = False
    else:
        try:
            subprocess.check_call(cmd, shell=True, stdout=logfile, stderr=logfile)
        except subprocess.CalledProcessError:
            ret = False
    return ret

def get_base_name(fpath):
    """This api takes rel path or full path and returns
    base name"""
    return os.path.basename(fpath.rstrip(os.path.sep))


def get_dir_path(fpath):
    """This api takes file path and returns it's directory
    path"""
    return os.path.dirname(fpath.rstrip(os.path.sep))

def get_abs_path(fpath):
    """This api takes file path and returns it's absolute
    path"""
    return os.path.abspath(fpath)


def get_original_path(fpath):
    """This api takes file path and returns it's original
    path. It is equivalent to readlink"""
    return os.path.realpath(fpath)

def find_file(search_file: str, search_path: str):
    """This api find the file in sub-directories and returns
    absolute path of file, if file exists"""
    for File in Path(search_path).glob(f"**/{search_file}"):
        return File

def find_files(search_pattern, search_path):
    """This api find the files matching regex directories and returns
    absolute path of files, if file exists"""

    return glob.glob(f"{search_path}/{search_pattern}")

