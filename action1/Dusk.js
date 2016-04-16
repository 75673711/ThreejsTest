//ydirection  1 向下曲面  2向上曲面
//lefth       左侧翘起高度
//righth      右侧翘起高度
//width       宽度
//height      长度
function WHDCreateLeaf(map, ydirection = 1, lefth = 5, righth = 8, width = 10, height = 20, xseg = 1, yseg = 20 )
{
    if ( typeof map === 'undefined' ) {
        console.log( "map is undefined" );
        return;
    }

    var curve = new THREE.CubicBezierCurve(
        new THREE.Vector3( 0, lefth, 0 ),
        new THREE.Vector3( 10, 0, 0 ),
        new THREE.Vector3( 13, 0, 0 ),
        new THREE.Vector3( 25, righth, 0 )
    );

    var geometry = new THREE.PlaneGeometry(width, height, xseg, yseg);
    for (var y = 0; y <= yseg; y++) {
        for (var x = 0; x <= xseg; x++) {
            geometry.vertices[y*(xseg+1)+x].z = curve.getPoint(y/yseg).y;
        }
    }
    geometry.center();

    var material = new THREE.MeshPhongMaterial( {
        specular: 0x030303,
        map: map,
        side: THREE.DoubleSide,
        alphaTest: 0.5
    } );

    var mesh = new THREE.Mesh( geometry, material );
    mesh.castShadow = true;
    mesh.receiveShadow  = true;
    mesh.rotation.y = Math.PI*3/2;
    mesh.lookAt ( new THREE.Vector3(0, 1*ydirection, 0) );

    return mesh;
}