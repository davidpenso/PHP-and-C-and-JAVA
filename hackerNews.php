
 <html>
 <head>
     <title></title>
 </head>

 <body>
    <h1>
        Hello,please vote <?php echo $_SESSION['username'];?> up or down.<br>
        For log out. <li><a href= 'logout.php'> Logout </a></li>
    </h1>
<form action="" method="post">
    <?php
    /**
     * Created by JetBrains PhpStorm.
     * User: david
     * Date: 6/6/13
     * Time: 10:30 AM
     * To change this template use File | Settings | File Templates.
     */

    session_start();

    $link = mysql_connect('localhost', 'root', '12345678');
    if (!$link) {
        die('Not connected : ' . mysql_error());
    }
    $db_selected = mysql_select_db('anketler');
    if (!$db_selected) {
        die ('Can\'t use foo : ' . mysql_error());
    }



    $result = mysql_query("SELECT * from  anket_seceneler  ");
    if (!$result) {
        die('Invalid query: ' . mysql_error());
    }
    else{
        while($datas=mysql_fetch_assoc($result)){

            echo ($datas['secenek']."<br><input type='radio' name='anketVoteID' value='".$datas['id']."'> like<br>");

        }

       echo '<input type="submit" value="submit" name="SubmitForm">';
        if(getpost($datas)) {
        };
            echo 'you re successfully voted';
            listle($datas);
    }






function getpost($datalar){

            if($_POST['anketVoteID']){

                $updated = mysql_query('update anket_seceneler set total_rate = total_rate + 1 where id = ' . $_POST['anketVoteID']);

            }


return $updated;
}


function listle($datalar){
    $result3 = mysql_query("SELECT * from  anket_seceneler order by total_rate DESC   ");
    if (!$result3) {
        die('Invalid query: ' . mysql_error());
    }
    else{
        echo $result3 ."<br><input type='radio' name='anketVoteID' value='".$datalar['id']."'> like<br>";

    }


}








    if(!is_logged()){
        header('Location: login.php');
        die();
    }



    function is_logged(){
        return isset($_SESSION['username']);
    }




    ?>




</form>




</body>



 </html>
