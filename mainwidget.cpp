/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwidget.h"
#include "transform.h"
#include "GameComponent.h"
#include "MeshRenderer.h"
#include "iostream"

#include <QMouseEvent>

#include <math.h>

MainWidget::MainWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    geometries(0),
    texture(0),
    angularSpeed(0),
    camera(),
    plan(),
    truc(),
    planRenderer(),
    trucRenderer()
{

    root = new GameObject();
    camera = new GameObject();
    plan = new GameObject();
    truc = new GameObject();
    cube2 = new GameObject();

    camPos = QVector3D(0.0f,0.0f,0.0f);
    center = QVector3D(0.0f,0.0f,0.0f),up = QVector3D(0.0f,1.0f,0.0f);
    camSpeed = 0.05f;

    //gestion saut
    inJump = false;
    inAir = false;
    cptSaut = 30;
    //view matrix

    yaw=0.0;
    pitch=0.0;
    // I assume the values are already converted to radians.
    float cosPitch = cos(pitch);
    float sinPitch = sin(pitch);
    float cosYaw = cos(yaw);
    float sinYaw = sin(yaw);
 
    QVector3D xaxis = QVector3D(cosYaw, 0, -sinYaw );
    QVector3D yaxis = QVector3D( sinYaw * sinPitch, cosPitch, cosYaw * sinPitch );
    QVector3D zaxis = QVector3D( sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw );
 
    // Create a 4x4 view matrix from the right, up, forward and eye position vectors
    view = QMatrix4x4(
        xaxis.x(),            yaxis.x(),            zaxis.x(), 0,
        xaxis.y(),            yaxis.y(),            zaxis.y(), 0,
        xaxis.z(),            yaxis.z(),            zaxis.z(), 0,
       -QVector3D::dotProduct( xaxis, camPos ), -QVector3D::dotProduct( yaxis, camPos ), -QVector3D::dotProduct( zaxis, camPos ), 1
    );
    ///////////////

    //view.lookAt(camPos,center,up);


    //init scene graph

    root->addEnfant(camera);
    root->addEnfant(plan);
    root->addEnfant(cube2);

    plan->addEnfant(truc);



    plan->localTransform.scale(QVector3D(4.0f, 4.0f,  4.0f));
    plan->localTransform.translate(QVector3D(-15.0, -3.0, -10.0));
    QQuaternion j = QQuaternion::fromAxisAndAngle(QVector3D(1,0,0), -90);
    plan->localTransform.rotate(j);
 
    truc->localTransform.translate(QVector3D(10.0,-10.0,10.0));

    cube2->localTransform.translate(QVector3D(10.0,0.0,10.0));

}

MainWidget::~MainWidget()
{
    // Make sure the context is current when deleting the texture
    // and the buffers.
    makeCurrent();
    delete texture;
    delete geometries;
    doneCurrent();
}

//! [0]
void MainWidget::mousePressEvent(QMouseEvent *e)
{
    // Save mouse press position
    mousePressPosition = QVector2D(e->localPos());
}

void MainWidget::mouseReleaseEvent(QMouseEvent *e)
{
    // Mouse release position - mouse press position
    QVector2D diff = QVector2D(e->localPos()) - mousePressPosition;

    // Rotation axis is perpendicular to the mouse position difference
    // vector
    QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();

    // Accelerate angular speed relative to the length of the mouse sweep
    qreal acc = diff.length() / 100.0;

    // Calculate new rotation axis as weighted sum
    rotationAxis = (rotationAxis * angularSpeed + n * acc).normalized();

    // Increase angular speed
    angularSpeed += acc;
}
//! [0]
void MainWidget::keyPressEvent(QKeyEvent *ev){
    switch ( ev->key() )
        {
            case Qt::Key_Escape :
                close();
                break;
            case Qt::Key_Z :

                translation.setX(translation.x() - view.column(2).x());
                translation.setY(translation.y() - view.column(2).y());
                translation.setZ(translation.z() + view.column(2).z());
        
                update();
                break;

            case Qt::Key_Q :
               
                yaw-=0.1;
                update();
                break;

            case Qt::Key_S :

                translation.setX(translation.x() + view.column(2).x());
                translation.setY(translation.y() + view.column(2).y());
                translation.setZ(translation.z() - view.column(2).z());

                update();
                break;

            case Qt::Key_D :
              
                yaw+=0.1;
                update();
                break;

            case Qt::Key_Up:

                translation.setX(translation.x() - view.column(1).x());
                translation.setY(translation.y() - view.column(1).y());
                translation.setZ(translation.z() + view.column(1).z());

                update();
                break;
            case Qt::Key_Down:

                translation.setX(translation.x() + view.column(1).x());
                translation.setY(translation.y() + view.column(1).y());
                translation.setZ(translation.z() - view.column(1).z());

                update();
                break;
            case Qt::Key_Right:
                
                translation.setX(translation.x() - view.column(0).x());
                translation.setY(translation.y() - view.column(0).y());
                translation.setZ(translation.z() + view.column(0).z());

                update();
                break;
            case Qt::Key_Left:

                translation.setX(translation.x() + view.column(0).x());
                translation.setY(translation.y() + view.column(0).y());
                translation.setZ(translation.z() - view.column(0).z());

                update();
                break;
            case Qt::Key_C :
                //mode orbital

                //orbital ^= true;
                //rester appuyer sur C
                /*
                if(orbital){
                    rotation = QQuaternion::fromAxisAndAngle(QVector3D(0,1,0), 5) * rotation;
                }
                */
                rotation = QQuaternion::fromAxisAndAngle(QVector3D(0,1,0), 5) * rotation;
                update();
                break;

             case Qt::Key_Space:
                    if(!inAir){
                        inJump = true;
                    }

    case Qt::Key_I :

        break;
        }
    //printf("\nkey event in board: %i", ev->key());

}
//! [1]
void MainWidget::timerEvent(QTimerEvent *)
{
    //update

    update();
    if(inJump){
        inAir = true;
        translation.setX(translation.x() - view.column(1).x());
        translation.setY(translation.y() - view.column(1).y());
        translation.setZ(translation.z() + view.column(1).z());

        cptSaut --;
        if(cptSaut <= 0){
            inJump = false;
            cptSaut = 30;
        }
    }
    else{
        if(inAir){
            translation.setX(translation.x() + view.column(1).x());
            translation.setY(translation.y() + view.column(1).y());
            translation.setZ(translation.z() - view.column(1).z());
            /*
            if(translation.y() <= 0){
                inAir = false;
            }
            */
        }

    }

    //
    // Decrease angular speed (friction)
    angularSpeed *= 0.99;

    // Stop rotation when speed goes below threshold
    if (angularSpeed < 0.01) {
        angularSpeed = 0.0;
    } else {
        // Update rotation
        rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;

        // Request an update
        update();
    }
}
//! [1]

void MainWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(0, 0, 0, 1);

    initShaders();
    initTextures();

//! [2]
    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);
//! [2]

    //geometries = new GeometryEngine;

    GameComponent* planRenderer=new MeshRenderer(30.0,30.0);
    plan->addComponent(planRenderer);
    GameComponent* trucRenderer=new MeshRenderer(1.0,1.0,1.0);
    truc->addComponent(trucRenderer);
    GameComponent* cube2Renderer=new MeshRenderer(5.0,5.0,5.0);
    cube2->addComponent(cube2Renderer);

    // Use QBasicTimer because its faster than QTimer
    timer.start(12, this);
}

//! [3]
void MainWidget::initShaders()
{
    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Link shader pipeline
    if (!program.link())
        close();

    // Bind shader pipeline for use
    if (!program.bind())
        close();
}
//! [3]

//! [4]
void MainWidget::initTextures()
{
    // Load cube.png image
    texture = new QOpenGLTexture(QImage(":/rock.png").mirrored());

    // Set nearest filtering mode for texture minification
    texture->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    texture->setWrapMode(QOpenGLTexture::Repeat);
}
//! [4]

//! [5]
void MainWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 1.0, zFar = 500.0, fov = 45.0;

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    projection.perspective(fov, aspect, zNear, zFar);


}
//! [5]

void MainWidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    texture->bind();

//! [6]
    // Calculate model view transformation
    QMatrix4x4 matrix;

    //view matrix

    // I assume the values are already converted to radians.
    float cosPitch = cos(pitch);
    float sinPitch = sin(pitch);
    float cosYaw = cos(yaw);
    float sinYaw = sin(yaw);
 
    QVector3D xaxis = { cosYaw, 0, -sinYaw };
    QVector3D yaxis = { sinYaw * sinPitch, cosPitch, cosYaw * sinPitch };
    QVector3D zaxis = { sinYaw * cosPitch, -sinPitch, cosPitch * cosYaw };
 
    // Create a 4x4 view matrix from the right, up, forward and eye position vectors
    view = QMatrix4x4(
        xaxis.x(),            yaxis.x(),            zaxis.x(), 0,
        xaxis.y(),            yaxis.y(),            zaxis.y(), 0,
        xaxis.z(),            yaxis.z(),            zaxis.z(), 0,
       -QVector3D::dotProduct( xaxis, camPos ), -QVector3D::dotProduct(yaxis, camPos), -QVector3D::dotProduct(zaxis, camPos), 1
    );

    view.translate(translation);

    ///////////////
    
    /*
    //chute et verif
    float vitGravite = 0.1;
    if(truc->getPos().at(2) > 1.0){
        truc->setPos(truc->getPos().at(0),truc->getPos().at(1),truc->getPos().at(2)-vitGravite);
    }
    */

    root->update();

    root->render(&program, projection, view);


}
