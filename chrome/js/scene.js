"use strict";

// Create a renderer using an existing canvas
var canvas = document.getElementById("canvas");
var renderer = new THREE.WebGLRenderer({ canvas: canvas });
var CANVAS_WIDTH = canvas.scrollWidth;
var CANVAS_HEIGHT = canvas.scrollHeight;
var CANVAS_ASPECT = CANVAS_WIDTH / CANVAS_HEIGHT;
renderer.setSize(CANVAS_WIDTH, CANVAS_HEIGHT);

// Create a scene.
var scene = new THREE.Scene();

// Create a camera
var camera = new THREE.PerspectiveCamera(30, CANVAS_ASPECT, 0.1, 1000);
camera.position.x = 1.4;
camera.position.y = -1.5;
camera.position.z = 1.5;
camera.up = new THREE.Vector3(0, 0, 1);
camera.lookAt(new THREE.Vector3(0.2, 0.2, 0.2));

// Create lights
var hemiLight;

hemiLight = new THREE.HemisphereLight(0xffffff, 0xffffff, 0.6);
hemiLight.color.setHSL(0.6, 1, 0.6);
hemiLight.groundColor.setHSL(0.095, 1, 0.75);
hemiLight.position.set(0, 100, 0);
scene.add(hemiLight);

// Add a cube to the scene
var material = new THREE.MeshPhongMaterial({color: 0xffff00,
                                        specular: 0xa0a0a0,
                                        shininess: 15,
                                        vertexColors: THREE.FaceColors,
                                        shading: THREE.FlatShading
                                       });
var materials = [
    new THREE.MeshLambertMaterial({
        map: THREE.ImageUtils.loadTexture('img/textures/Right.png')
    }),
    new THREE.MeshLambertMaterial({
        map: THREE.ImageUtils.loadTexture('img/textures/Left.png')
    }),
    new THREE.MeshLambertMaterial({
        map: THREE.ImageUtils.loadTexture('img/textures/Back.png')
    }),
    new THREE.MeshLambertMaterial({
        map: THREE.ImageUtils.loadTexture('img/textures/Front.png')
    }),
    new THREE.MeshLambertMaterial({
        map: THREE.ImageUtils.loadTexture('img/textures/Top.png')
    }),
    new THREE.MeshLambertMaterial({
        map: THREE.ImageUtils.loadTexture('img/textures/Bottom.png')
    }),
];
material = new THREE.MeshFaceMaterial(materials);
var cube = new THREE.Mesh(new THREE.BoxGeometry(1.0, 0.6, 0.3), material);
cube.position.x = 0.2;
cube.position.y = 0.2;
cube.position.z = 0.2;
scene.add(cube);

// Render function.
var render = function () {
    requestAnimationFrame(render);
    renderer.render(scene, camera);
};

render();

function onWindowResize() {
    var canvas = document.getElementById('canvas');
    CANVAS_WIDTH = canvas.offsetWidth;
    CANVAS_HEIGHT = canvas.offsetHeight;
    //console.log("WIDTH", CANVAS_WIDTH);
    //console.log("HEIGHT", CANVAS_HEIGHT);
    camera.aspect = CANVAS_WIDTH / CANVAS_HEIGHT;
    camera.updateProjectionMatrix();
    renderer.setSize(CANVAS_WIDTH, CANVAS_HEIGHT);
}


window.addEventListener('resize', onWindowResize, false);

var gx = 0.5;
var gy = 0.0;
var gz = 0.0;
var ax = 0.0;
var ay = 0.0;
var az = 1.0;
var mx = 0.0;
var my = 0.0;
var mz = 0.0;

var samplePeriodMillis = 1000 / sampleFreq;

function scene_update() {

    ax = imuData.a[0];
    ay = imuData.a[1];
    az = imuData.a[2];
    gx = imuData.g[0] / 4;
    gy = imuData.g[1] / 4;
    gz = imuData.g[2] / 4;
    mx = imuData.m[0];
    my = imuData.m[1];
    mz = imuData.m[2];

    mx = 0.0;
    my = 0.0;
    mz = 0.0;
    madgwickAHRSupdate(gx, gy, gz, ax, ay, az, mx, my, mz);
    cube.quaternion.set(q1, q2, q3, q0);

    if ((q0 === NaN) || (q1 === NaN) || (q2 === NaN) || (q3 === NaN)) {
        console.log ("Exploded!");
    }
}

// Find intersections
var projector = new THREE.Projector();
var count = 0;

setInterval(function () {
    scene_update();
}, samplePeriodMillis);
