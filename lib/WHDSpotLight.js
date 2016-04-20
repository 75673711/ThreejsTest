//radians 光照垂线与边夹角  length 光护罩长度 shadowMapLength 光照产生阴影的最长距离 light 灯光颜色 colorDecay 光护罩颜色衰弱 1.0-0.0
THREE.WHDSpotLight = function ( scene, radians, length, shadowMapLength, lightColor, colorDecay ) {

    radians = radians == undefined ? Math.PI/8 : radians;
    length = length == undefined ? 400.0 : length;
    shadowMapLength = shadowMapLength == undefined ? 800 : shadowMapLength;
    lightColor = lightColor == undefined ? 0x0000ff : lightColor;
    colorDecay = colorDecay == undefined ? 0.5 : colorDecay;
    colorDecay = 1 - colorDecay;
    
    //light
    //颜色 强度 距离 角度rad 中心垂线到边的角度 exponent, decay
    var spotLight = new THREE.SpotLight(lightColor, 1, length, radians );
    spotLight.castShadow = true;

    spotLight.shadow.mapSize.width = 1024;
    spotLight.shadow.mapSize.height = 1024;

    spotLight.shadow.camera.near = 50;
    spotLight.shadow.camera.far = shadowMapLength;
    //114.59155902616465   0.00872665  灯光角度两倍
    spotLight.shadow.camera.fov = 114.59155902616465*radians;

    scene.add(spotLight);
    scene.add(spotLight.target);

    this.spotLight = spotLight;

    var cameraHelper = new THREE.CameraHelper(this.spotLight.shadow.camera);
    scene.add(cameraHelper);

    //cylinder
    //(radiusTop, radiusBottom, height, radiusSegments, heightSegments, openEnded, thetaStart, thetaLength)
    var cylinder = new THREE.CylinderGeometry(length*Math.tan(radians), 0, length, 100, 100, true);
    cylinder.translate( 0, length/2, 0);
    cylinder.rotateX(Math.PI/2);

    var vertextShaderText = "varying vec2 vUv;"+
        "void main() {"+
        "vUv = uv;"+
        "gl_Position = projectionMatrix * modelViewMatrix * vec4(position,1.0);"+
        "}";

    var fragmentShaderText = "varying vec2 vUv;"+
        "uniform vec3 myColor;"+
        "uniform float colorDecay;"+
        "void main() {"+
        "if(vUv[1] < 0.05) discard;"+
        "gl_FragColor = vec4( vUv[1]+myColor, colorDecay*(1.0-vUv[1]) );"+
        "}";
//vec4( vUv[1]+myColor, 0.5*(1.0-vUv[1]) )
    var material = new THREE.ShaderMaterial({
        uniforms: {
            myColor: { type: "c", value: new THREE.Color( lightColor ) },
            colorDecay: { type: "f", value: colorDecay }
        },
        vertexShader: vertextShaderText,
        fragmentShader: fragmentShaderText
    });
    material.transparent = true;
    material.depthWrite = false;
    material.depthTest = true;
    material.side = THREE.DoubleSide;

    this.lightCylinder = new THREE.Mesh( cylinder, material );
    scene.add( this.lightCylinder );

    //bulb
    var circle = new THREE.CircleGeometry( length*Math.tan(radians)/20, 32 );
    var mirMaterial = new THREE.MeshBasicMaterial( {color: 0x4444ff} );
    mirMaterial.side = THREE.DoubleSide;
    this.bulb = new THREE.Mesh( circle, mirMaterial );
    this.bulb.translateZ(length/20);
    this.lightCylinder.add(this.bulb);

    this.update = function ( targetPosition ) {
        this.spotLight.target.position.copy( targetPosition );

        var p1 = this.spotLight.target.position;
        var p2 = this.spotLight.position;
        var dir = new THREE.Vector3( p1.x-p2.x, p1.y-p2.y, p1.z-p2.z );

        this.lightCylinder.position.set( 0, 0, 0 );
        this.lightCylinder.lookAt( dir );
        this.lightCylinder.position.copy( p2 );
    }

    this.setPosition = function ( x, y, z ) {
        this.spotLight.position.set( x, y, z);
    }
}
