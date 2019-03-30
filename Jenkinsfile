node {
    stage('Preparation') {
        git 'https://github.com/UoLeevi/projectfina-api.git'
    }
    stage('Build') {
        sh 'sudo scripts/rebuild.sh'
    }
    stage('Restart') {
        sh 'sudo systemctl restart projectfina-api.service'
    }
}
