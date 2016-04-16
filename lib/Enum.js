/**
 * Created by adminm on 2016/4/9.
 */
var RIGHTFACE1 = 0;
var RIGHTFACE2 = 1;
var LEFTFACE1 = 2;
var LEFTFACE2 = 3;
var TOPFACE1 = 4;
var TOPFACE2 = 5;
var BOTTOMFACE1 = 6;
var BOTTOMFACE2 = 7;
var FRONTFACE1 = 8;
var FRONTFACE2 = 9;
var BACKFACE1 = 10;
var BACKFACE2 = 11;
var BOXGEOMETRYFACESLENGTH = 12;

var BLUE = 0x0000ff;   //right
var GREEN = 0X00ff00;  //left
var RED = 0xff0000;    //top
var WHITE = 0xffffff;  //bottom
var CYAN = 0x00ffff;   //front
var YELLOW = 0xffff00; //back
var DARK= 0xcdcdcd;

var VERTICAL = 2;
var HORIZONTAL = 1;
var NODIRECTION = 0;

var INVALIDPOSITION = 0;
var FRONTPOSITION = 1;
var BACKPOSITION = 2;
var RIGHTPOSITION = 3;
var LEFTPOSITION = 4;
var TOPPOSITION = 5;
var BOTTOMPOSITION = 6;

function colorArrayWithFace(right, left, top, bottom, front, back) {
    var colorArray = new Array();
    colorArray.push(right);
    colorArray.push(right);
    colorArray.push(left);
    colorArray.push(left);
    colorArray.push(top);
    colorArray.push(top);
    colorArray.push(bottom);
    colorArray.push(bottom);
    colorArray.push(front);
    colorArray.push(front);
    colorArray.push(back);
    colorArray.push(back);
    return colorArray;
}

function initAxis(scene) {
    drawLine(new THREE.Vector3( -1000, 0, 0 ), new THREE.Vector3( 1000, 0, 0 ), RED, scene);
    drawLine(new THREE.Vector3( 0, -1000, 0 ), new THREE.Vector3( 0, 1000, 0 ), BLUE, scene);
    drawLine(new THREE.Vector3( 0, 0, -1000 ), new THREE.Vector3( 0, 0, 1000 ), GREEN, scene);
}

function drawLine(fromPoint, toPoint, color, scene) {
    var material = new THREE.LineBasicMaterial({
        color: color
    });
    //线段与平面重叠时交叉渲染
    // material.depthWrite = false;
    // material.depthTest = false;

    var geometry = new THREE.Geometry();
    geometry.vertices.push(
        fromPoint,
        toPoint
    );

    var line = new THREE.Line( geometry, material );
    scene.add( line );
}

function turnRoundObject(object, radius) {
    var time = Date.now() * 0.0005;
    object.position.x = Math.sin( time ) * radius;  
    object.position.z = Math.cos( time ) * radius;
    //object.position.y = Math.cos( time ) * radius;
}

function initBridgeCube(scene) {
    var r = "../images/Bridge2/";
    var urls = [
        r + "posx.jpg", r + "negx.jpg",
        r + "posy.jpg", r + "negy.jpg",
        r + "posz.jpg", r + "negz.jpg"
    ];

    var textureCube = new THREE.CubeTextureLoader().load( urls );
    //textureCube.mapping = THREE.CubeRefractionMapping;

    // Skybox

    var shader = THREE.ShaderLib[ "cube" ];
    shader.uniforms[ "tCube" ].value = textureCube;

    var material = new THREE.ShaderMaterial( {

            fragmentShader: shader.fragmentShader,
            vertexShader: shader.vertexShader,
            uniforms: shader.uniforms,
            side: THREE.BackSide

        } ), 
        mesh = new THREE.Mesh( new THREE.BoxGeometry( 100000, 100000, 100000 ), material );
    scene.add( mesh );
}

function initRing(scene) {
    var material = new THREE.MeshNormalMaterial();  //{ side: THREE.DoubleSide } 

    object = new THREE.Mesh( new THREE.RingGeometry( 10, 15, 200, 5, 0, Math.PI * 2 ), material );
    //object = new THREE.Mesh( new THREE.RingGeometry( 5, 10, 32 ), material);
    object.position.set( 0, 0, 0 );
    object.rotation.x = -90*Math.PI/180;

    scene.add( object );
}

function initBox(scene) {
    var material = new THREE.MeshPhongMaterial( {color: 0xff0000} );

    var mesh = new THREE.Mesh(new THREE.BoxGeometry(100, 100, 100), material);
    mesh.castShadow = true;
    mesh.receiveShadow = true;
    scene.add(mesh);
}