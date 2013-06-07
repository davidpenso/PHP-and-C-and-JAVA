<?php
/**
 * Created by JetBrains PhpStorm.
 * User: david
 * Date: 6/6/13
 * Time: 10:44 AM
 * To change this template use File | Settings | File Templates.
 */
session_start();

session_destroy();

$_SESSION= array();

header('Location: login.php');

